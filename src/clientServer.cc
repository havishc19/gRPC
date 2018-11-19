#include <memory>
#include <iostream>
#include <string>
#include <thread>
#include <vector>
#include <chrono>

#include "threadpool.h"
#include "vendorClient.cc"


using namespace std;

#include <grpcpp/grpcpp.h>
#include <grpc/support/log.h>

#include "store.grpc.pb.h"

using grpc::Server;
using grpc::ServerAsyncResponseWriter;
using grpc::ServerBuilder;
using grpc::ServerContext;
using grpc::ServerCompletionQueue;
using grpc::Status;


using store::ProductQuery;
using store::ProductReply;
using store::ProductInfo;
using store::Store;

vector<string> vendorAddresses;

threadpool pool;
class ClientServer final {
 public:

  ~ClientServer() {
    server_->Shutdown();
    cq_->Shutdown();
  }

  void print(std::vector<std::string> A){
    for(int i=0;i<A.size();i++){
      cout << A[i] << endl;
    }
  }

  void Run(string IP_ADDR_PORT) {
    string server_address(IP_ADDR_PORT);
    ServerBuilder builder;
    builder.AddListeningPort(server_address, grpc::InsecureServerCredentials());
    builder.RegisterService(&service_);
    cq_ = builder.AddCompletionQueue();
    server_ = builder.BuildAndStart();
    cout << "Server listening on " << server_address << endl;
    HandleRpcs();
  }


 private:
  class CallData {
   public:
    CallData(Store::AsyncService* service, ServerCompletionQueue* cq)
        : service_(service), cq_(cq), responder_(&ctx_), status_(CREATE) {
      Proceed();
    }
        void printThreadInfo(std::thread::id id, string productName){
          cout << id << " Serving Request for product " << productName << endl;
        }
        
        void Proceed() {
      if (status_ == CREATE) {
        status_ = PROCESS;
        service_->RequestgetProducts(&ctx_, &request_, &responder_, cq_, cq_,
                                  this);
      } else if (status_ == PROCESS) {
        pool.addJob( [this]() { 
            // std::thread::id this_id = std::this_thread::get_id();
            new CallData(service_, cq_);
            string productName = request_.product_name();

            // printThreadInfo(this_id, productName);

            for(int i=0;i<vendorAddresses.size();i++){
              std::vector<string> vendorResponse = queryVendor(vendorAddresses[i], productName);
              ProductInfo *product = reply_.add_products();
              product->set_price(stod(vendorResponse[0]));
              product->set_vendor_id(vendorResponse[1]);
            }

            status_ = FINISH;
            responder_.Finish(reply_, Status::OK, this);
        });
      } else {
        GPR_ASSERT(status_ == FINISH);
        delete this;
      }
    }

   private:
    Store::AsyncService* service_;
    ServerCompletionQueue* cq_;
    ServerContext ctx_;
    ProductQuery request_;
    ProductReply reply_;
    ServerAsyncResponseWriter<ProductReply> responder_;
    enum CallStatus { CREATE, PROCESS, FINISH };
    CallStatus status_;
  };

  void HandleRpcs() {
    new CallData(&service_, cq_.get());
    void* tag;  
    bool ok;
    while (true) {
      GPR_ASSERT(cq_->Next(&tag, &ok));
      GPR_ASSERT(ok);
      static_cast<CallData*>(tag)->Proceed();
    }
  }

  unique_ptr<ServerCompletionQueue> cq_;
  Store::AsyncService service_;
  unique_ptr<Server> server_;
};

int launchServer(vector<string> vendorArgs, string IP_ADDR_PORT, int maxThreads) {
  pool.setThreadPool(maxThreads);
  vendorAddresses = vendorArgs;
  ClientServer server;
  server.Run(IP_ADDR_PORT);

  return 0;
}
