#include <iostream>
#include <memory>
#include <string>
using namespace std;

#include <grpcpp/grpcpp.h>
#include <grpc/support/log.h>

#include "vendor.grpc.pb.h"

using grpc::Channel;
using grpc::ClientAsyncResponseReader;
using grpc::ClientContext;
using grpc::CompletionQueue;
using grpc::Status;
using vendor::Vendor;
using vendor::BidQuery;
using vendor::BidReply;

class VendorClient {
 public:

  explicit VendorClient(shared_ptr<Channel> channel)
      : stub_(Vendor::NewStub(channel)) {}

  vector<string> bidQuery(string IP_ADDR_HOST, string productName) {

    BidQuery request;
    request.set_product_name(productName);
    BidReply reply;
    ClientContext context;
    CompletionQueue cq;
    Status status;

    unique_ptr<ClientAsyncResponseReader<BidReply> > rpc(
        stub_->PrepareAsyncgetProductBid(&context, request, &cq));

    rpc->StartCall();
    rpc->Finish(&reply, &status, (void*)1);
    void* got_tag;
    bool ok = false;

    GPR_ASSERT(cq.Next(&got_tag, &ok));
    GPR_ASSERT(got_tag == (void*)1);
    GPR_ASSERT(ok);

    vector<string> returnVals; returnVals.resize(2);
    if (status.ok()) {
      returnVals[0] = to_string(reply.price());
      returnVals[1] = reply.vendor_id();
      return returnVals;
    } else {
      returnVals[0] = "NA";
      returnVals[1] = "NA - RPC Failed";
      return returnVals;
    }
  }

 private:
  unique_ptr<Vendor::Stub> stub_;
};

void printReply(vector<string> reply){
  cout << "Price: " << reply[0] << endl;
  cout << "Vendor ID: " << reply[1] << endl;
}

vector<string> queryVendor(string IP_ADDR_HOST, string productName) {
  VendorClient vendorClient(grpc::CreateChannel(
      IP_ADDR_HOST, grpc::InsecureChannelCredentials()));
  vector<string> reply = vendorClient.bidQuery(IP_ADDR_HOST, productName); 
  // printReply(reply);

  return reply;
}
