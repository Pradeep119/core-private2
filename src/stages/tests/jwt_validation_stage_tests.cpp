/*
 *
 *   © Copyrights 2021 Axiata Digital Labs Pvt Ltd.
 *   All Rights Reserved.
 *
 *   These material are unpublished, proprietary, confidential source
 *   code of Axiata Digital Labs Pvt Ltd (ADL) and constitute a TRADE
 *   SECRET of ADL.
 *
 *   ADL retains all title to and intellectual property rights in these
 *   materials.
 *
 *
 *   @author Braveenan Sritharan (braveenan.sritharan@axiatadigitallabs.com) on 2021/12/23.
 *
 */

#include <gtest/gtest.h>
#include "../../stages/jwt_validation_stage/jwt_validation_stage.h"

namespace adl::axp::core::stages::jwt_validation_stage::test {

/**Message collector class*/
namespace {
class MessageCollectorSink : public event_processing::ISink {
 public:
  MessageCollectorSink(const std::string &name) : _name(name) {
  }

  void on_message(event_processing::IMessage *message) noexcept override {
    _received_messages.emplace_back(message);
    executed = true;
  }

/**This will return "wrong" if the on_message of this sink is not hit*/
  std::string get_input_source_name() {
    if (executed) {
      executed = false;
      return _name;
    }
    return "wrong";
  }

  std::vector<event_processing::IMessage *> get_received_messages() {
    return _received_messages;
  }

 private:
  std::vector<event_processing::IMessage *> _received_messages;
  const std::string &_name;
  bool executed = false;
};
}

TEST(jwt_token, valid_jwt_token_validation) {
  std::string rsa_pub_key = R"(-----BEGIN PUBLIC KEY-----
MIIBIjANBgkqhkiG9w0BAQEFAAOCAQ8AMIIBCgKCAQEAxeqoZYbQ/Sr8DOFQ+/qb
EbCp6Vzb5hzH7oa3hf2FZxRKF0H6b8COMzz8+0mvEdYVvb/31jMEL2CIQhkQRol1
IruD6nBOmkjuXJSBficklMaJZORhuCrB4roHxzoG19aWmscA0gnfBKo2oGXSjJmn
ZxIh+2X6syHCfyMZZ00LzDyrgoXWQXyFvCA2ax54s7sKiHOM3P4A9W4QUwmoEi4H
QmPgJjIM4eGVPh0GtIANN+BOQ1KkUI7OzteHCTLu3VjxM0sw8QRayZdhniPF+U9n
3fa1mO4KLBsW4mDLjg8R/JuAGTX/SEEGj0B5HWQAP6myxKFz2xwDaCGvT+rdvkkt
OwIDAQAB
-----END PUBLIC KEY-----)";

  std::string token1 =
      "eyJ4NXQiOiJNell4TW1Ga09HWXdNV0kwWldObU5EY3hOR1l3WW1NNFpUQTNNV0kyTkRBelpHUXpOR00wWkdSbE5qSmtPREZrWkRSaU9URmtNV0ZoTXpVMlpHVmxOZyIsImtpZCI6Ik16WXhNbUZrT0dZd01XSTBaV05tTkRjeE5HWXdZbU00WlRBM01XSTJOREF6WkdRek5HTTBaR1JsTmpKa09ERmtaRFJpT1RGa01XRmhNelUyWkdWbE5nX1JTMjU2IiwiYWxnIjoiUlMyNTYifQ.eyJzdWIiOiJhZG1pbiIsImF1dCI6IkFQUExJQ0FUSU9OIiwiYXVkIjoiNDNSTllaZVh4YzJNaFpYZEdRX1djalBJb3FnYSIsIm5iZiI6MTY0MTgxODQ1MCwiYXpwIjoiNDNSTllaZVh4YzJNaFpYZEdRX1djalBJb3FnYSIsImlzcyI6Imh0dHBzOlwvXC9sb2NhbGhvc3Q6OTQ0M1wvb2F1dGgyXC90b2tlbiIsImV4cCI6MTY0NDQ5Njg1MCwiaWF0IjoxNjQxODE4NDUwLCJqdGkiOiI2NjZlMjEyYy1iMzM3LTRmMjEtODc2OC00NzRlMmU2NGFjZTUifQ.kFzG8w4lX7KK1i5qcppuj6-lGAWwiH-V-gsFbLOExtFaW7OnkYB8bdPoH8oPlYh5tR1QQnPdB-BwVQurHmxEykqQnUO-iwlJspxdPxd35xIBYiZNCqmAVme4j7PSb-Sub1bJeah4qhDMJ28foJJlgomdpNttraNaYpduY3EhUy8F39s8m-F1xAXxUArAeQL0YKI6l38aIxlUX6Vw492nzXCLpq7MOS3OA0bYbLqt-e9t5E_HK1NRRRfZ71wGcR1pzIocdGFPkPhjWqO5zKoM8DXRHTuCp5aaxPA55dHZAFlCDSZbIcEPWm2ILAMpNxntDXbiJPYIXMsPCGL_NJHOvw";

  stages::jwt_token::JwtValidationStage::field_combination_t token_field = {"http_info", "authorization"};

  stages::jwt_token::JwtValidationStage
      jwt_validate_stage{"valid_jwt_token_validation", token_field, "token_info", rsa_pub_key};

  event_processing::DynamicMessage dynamic_message_1;

  MessageCollectorSink valid_out{"valid_out"};
  MessageCollectorSink invalid_out{"invalid_out"};
  MessageCollectorSink expired_out{"expired_out"};
  MessageCollectorSink non_jwt_out{"non_jwt_out"};

  jwt_validate_stage.get_source("valid_out")->set_sink(&valid_out);
  jwt_validate_stage.get_source("invalid_out")->set_sink(&invalid_out);
  jwt_validate_stage.get_source("expired_out")->set_sink(&expired_out);
  jwt_validate_stage.get_source("non_jwt_out")->set_sink(&non_jwt_out);

  dynamic_message_1.set_field<std::string>("http_info", "authorization", token1);

  EXPECT_NO_THROW(jwt_validate_stage.get_sink("message_in")->on_message(&dynamic_message_1));

  EXPECT_EQ(1, valid_out.get_received_messages().size());
  EXPECT_EQ(0, invalid_out.get_received_messages().size());
  EXPECT_EQ(0, expired_out.get_received_messages().size());
  EXPECT_EQ(0, non_jwt_out.get_received_messages().size());
}

TEST(jwt_token, expired_token_validation) {
  std::string rsa_pub_key = R"(-----BEGIN PUBLIC KEY-----
MIIBIjANBgkqhkiG9w0BAQEFAAOCAQ8AMIIBCgKCAQEAxeqoZYbQ/Sr8DOFQ+/qb
EbCp6Vzb5hzH7oa3hf2FZxRKF0H6b8COMzz8+0mvEdYVvb/31jMEL2CIQhkQRol1
IruD6nBOmkjuXJSBficklMaJZORhuCrB4roHxzoG19aWmscA0gnfBKo2oGXSjJmn
ZxIh+2X6syHCfyMZZ00LzDyrgoXWQXyFvCA2ax54s7sKiHOM3P4A9W4QUwmoEi4H
QmPgJjIM4eGVPh0GtIANN+BOQ1KkUI7OzteHCTLu3VjxM0sw8QRayZdhniPF+U9n
3fa1mO4KLBsW4mDLjg8R/JuAGTX/SEEGj0B5HWQAP6myxKFz2xwDaCGvT+rdvkkt
OwIDAQAB
-----END PUBLIC KEY-----)";

  std::string token2 =
      "eyJ4NXQiOiJNell4TW1Ga09HWXdNV0kwWldObU5EY3hOR1l3WW1NNFpUQTNNV0kyTkRBelpHUXpOR00wWkdSbE5qSmtPREZrWkRSaU9URmtNV0ZoTXpVMlpHVmxOZyIsImtpZCI6Ik16WXhNbUZrT0dZd01XSTBaV05tTkRjeE5HWXdZbU00WlRBM01XSTJOREF6WkdRek5HTTBaR1JsTmpKa09ERmtaRFJpT1RGa01XRmhNelUyWkdWbE5nX1JTMjU2IiwiYWxnIjoiUlMyNTYifQ.eyJzdWIiOiJhZG1pbiIsImF1dCI6IkFQUExJQ0FUSU9OIiwiYXVkIjoieUFxbzh1WG1uM2dlelY4aVZqUTFJU1RTSmI4YSIsIm5iZiI6MTY0MTgxODY0OCwiYXpwIjoieUFxbzh1WG1uM2dlelY4aVZqUTFJU1RTSmI4YSIsImlzcyI6Imh0dHBzOlwvXC9sb2NhbGhvc3Q6OTQ0M1wvb2F1dGgyXC90b2tlbiIsImV4cCI6MTY0MTgxODcwOCwiaWF0IjoxNjQxODE4NjQ4LCJqdGkiOiIzYmZiZWNlZC1hMTE1LTQxZGQtYjFiZi0zZTI0MzBlNTM4NTgifQ.djfMED8ROrbh0YIjJK1TnHF6u3daCpnRMWzviUT3470FnPiWU1efp7RfehrPD4yaTKlulKyAa_W4EuumPdIIc9DdygOniABrSQamSIVDm39dbsZ9i4QgTuQGsB2Y8K8SgTNr82xnW_gWSoVpwMAsbjqwHE9XBP8ywMIvgZloKD2KRnHbscDyRaqvZjmrPe2yYUS4u6hPYRTcHdaN6Zl_qH7CLTNlRySeZ8csGcBsNO_d96XTn8UNOGYSZc35UQ3MiOxIo49jFNpwQkPdvxppf0gn9zutesEAauLCR-NlkrJXH9PKy1UcGfDc_-uKXd5t4DRyDL79zaxhnjp3gW-1Dg";

  stages::jwt_token::JwtValidationStage::field_combination_t token_field = {"http_info", "authorization"};

  stages::jwt_token::JwtValidationStage
      jwt_validate_stage{"expired_token_validation", token_field, "token_info", rsa_pub_key};

  event_processing::DynamicMessage dynamic_message_2;

  MessageCollectorSink valid_out{"valid_out"};
  MessageCollectorSink invalid_out{"invalid_out"};
  MessageCollectorSink expired_out{"expired_out"};
  MessageCollectorSink non_jwt_out{"non_jwt_out"};

  jwt_validate_stage.get_source("valid_out")->set_sink(&valid_out);
  jwt_validate_stage.get_source("invalid_out")->set_sink(&invalid_out);
  jwt_validate_stage.get_source("expired_out")->set_sink(&expired_out);
  jwt_validate_stage.get_source("non_jwt_out")->set_sink(&non_jwt_out);

  dynamic_message_2.set_field<std::string>("http_info", "authorization", token2);

  EXPECT_NO_THROW(jwt_validate_stage.get_sink("message_in")->on_message(&dynamic_message_2));

  EXPECT_EQ(0, valid_out.get_received_messages().size());
  EXPECT_EQ(0, invalid_out.get_received_messages().size());
  EXPECT_EQ(1, expired_out.get_received_messages().size());
  EXPECT_EQ(0, non_jwt_out.get_received_messages().size());
}

TEST(jwt_token, non_jwt_token_validation) {
  std::string rsa_pub_key = R"(-----BEGIN PUBLIC KEY-----
MIIBIjANBgkqhkiG9w0BAQEFAAOCAQ8AMIIBCgKCAQEAxeqoZYbQ/Sr8DOFQ+/qb
EbCp6Vzb5hzH7oa3hf2FZxRKF0H6b8COMzz8+0mvEdYVvb/31jMEL2CIQhkQRol1
IruD6nBOmkjuXJSBficklMaJZORhuCrB4roHxzoG19aWmscA0gnfBKo2oGXSjJmn
ZxIh+2X6syHCfyMZZ00LzDyrgoXWQXyFvCA2ax54s7sKiHOM3P4A9W4QUwmoEi4H
QmPgJjIM4eGVPh0GtIANN+BOQ1KkUI7OzteHCTLu3VjxM0sw8QRayZdhniPF+U9n
3fa1mO4KLBsW4mDLjg8R/JuAGTX/SEEGj0B5HWQAP6myxKFz2xwDaCGvT+rdvkkt
OwIDAQAB
-----END PUBLIC KEY-----)";

  std::string token3 =
      "e3631361-f45a-39e6-9b76-d051619d0402";

  stages::jwt_token::JwtValidationStage::field_combination_t token_field = {"http_info", "authorization"};

  stages::jwt_token::JwtValidationStage
      jwt_validate_stage{"non_jwt_token_validation", token_field, "token_info", rsa_pub_key};

  event_processing::DynamicMessage dynamic_message_3;

  MessageCollectorSink valid_out{"valid_out"};
  MessageCollectorSink invalid_out{"invalid_out"};
  MessageCollectorSink expired_out{"expired_out"};
  MessageCollectorSink non_jwt_out{"non_jwt_out"};

  jwt_validate_stage.get_source("valid_out")->set_sink(&valid_out);
  jwt_validate_stage.get_source("invalid_out")->set_sink(&invalid_out);
  jwt_validate_stage.get_source("expired_out")->set_sink(&expired_out);
  jwt_validate_stage.get_source("non_jwt_out")->set_sink(&non_jwt_out);

  dynamic_message_3.set_field<std::string>("http_info", "authorization", token3);

  EXPECT_NO_THROW(jwt_validate_stage.get_sink("message_in")->on_message(&dynamic_message_3));

  EXPECT_EQ(0, valid_out.get_received_messages().size());
  EXPECT_EQ(0, invalid_out.get_received_messages().size());
  EXPECT_EQ(0, expired_out.get_received_messages().size());
  EXPECT_EQ(1, non_jwt_out.get_received_messages().size());
}

TEST(jwt_token, invalid_jwt_token_validation) {
  std::string rsa_pub_key = R"(-----BEGIN PUBLIC KEY-----
MIIBIjANBgkqhkiG9w0BAQEFAAOCAQ8AMIIBCgKCAQEAxeqoZYbQ/Sr8DOFQ+/qb
EbCp6Vzb5hzH7oa3hf2FZxRKF0H6b8COMzz8+0mvEdYVvb/31jMEL2CIQhkQRol1
IruD6nBOmkjuXJSBficklMaJZORhuCrB4roHxzoG19aWmscA0gnfBKo2oGXSjJmn
ZxIh+2X6syHCfyMZZ00LzDyrgoXWQXyFvCA2ax54s7sKiHOM3P4A9W4QUwmoEi4H
QmPgJjIM4eGVPh0GtIANN+BOQ1KkUI7OzteHCTLu3VjxM0sw8QRayZdhniPF+U9n
3fa1mO4KLBsW4mDLjg8R/JuAGTX/SEEGj0B5HWQAP6myxKFz2xwDaCGvT+rdvkkt
OwIDAQAB
-----END PUBLIC KEY-----)";

  std::string token4 =
      "eyJ4NXQiOiJNell4TW1Ga09HWXdNV0kwWldObU5EY3hOR1l3WW1NNFpUQTNNV0kyTkRBelpHUXpOR00wWkdSbE5qSmtPREZrWkRSaU9URmtNV0ZoTXpVMlpHVmxOZyIsImtpZCI6Ik16WXhNbUZrT0dZd01XSTBaV05tTkRjeE5HWXdZbU00WlRBM01XSTJOREF6WkdRek5HTTBaR1JsTmpKa09ERmtaRFJpT1RGa01XRmhNelUyWkdWbE5nX1JTMjU2IiwiYWxnIjoiUlMyNTYifQ.eyJzdWIiOiJhZG1pbiIsImF1dCI6IkFQUExJQ0FUSU9OIiwiYXVkIjoiNDNSTllaZVh4YzJNaFpYZEdRX1djalBJb3FnYSIsIm5iZiI6MTY0MDAwMjkyNywiYXpwIjoiNDNSTllaZVh4YzJNaFpYZEdRX1djalBJb3FnYSIsImlzcyI6Imh0dHBzOlwvXC9sb2NhbGhvc3Q6OTQ0M1wvb2F1dGgyXC90b2tlbiIsImV4cCI6MTY0MjY4MTMyNywiaWF0IjoxNjQwMDAyOTI3LCJqdGkiOiI1ZmI4MWFhOC0xOTBkLTQ5MWItYTIwMC0wZTA4YzA5YTc1MGQifQ.QcZ3PEa-C-I9rq5uZ_7B7HK6xQH8dQ3Rv7momX9CML_YTybvYhQ-jhoTP4bB7KU2SyW2pyQS3PXHnZov_SzU0-QhxzTZJGLEjRImo4W-o0YwI8k4S5t-kc5eRsx5cEQiFd5r-aeIsCrfyVEC-rFzuKWGx9j2VtLCVqab62ZXUpgnjEmLk0qL6An-yhVQtMiqrYrSs-QM3JYxBUZriiBgijMsmYBV1_15E5y-s55nDEXo4btRumPqdUyS-_pp2XzMdMEOqnYrTDsORYKOYBc_jEERx2aWs6EMUkjGozad518K1MO3EMh9cMul1iqWLN4jy7YVkUISei1o4LkCnmQ0yg";

  stages::jwt_token::JwtValidationStage::field_combination_t token_field = {"http_info", "authorization"};

  stages::jwt_token::JwtValidationStage
      jwt_validate_stage{"invalid_jwt_token_validation", token_field, "token_info", rsa_pub_key};

  event_processing::DynamicMessage dynamic_message_4;

  MessageCollectorSink valid_out{"valid_out"};
  MessageCollectorSink invalid_out{"invalid_out"};
  MessageCollectorSink expired_out{"expired_out"};
  MessageCollectorSink non_jwt_out{"non_jwt_out"};

  jwt_validate_stage.get_source("valid_out")->set_sink(&valid_out);
  jwt_validate_stage.get_source("invalid_out")->set_sink(&invalid_out);
  jwt_validate_stage.get_source("expired_out")->set_sink(&expired_out);
  jwt_validate_stage.get_source("non_jwt_out")->set_sink(&non_jwt_out);

  dynamic_message_4.set_field<std::string>("http_info", "authorization", token4);

  EXPECT_NO_THROW(jwt_validate_stage.get_sink("message_in")->on_message(&dynamic_message_4));

  EXPECT_EQ(0, valid_out.get_received_messages().size());
  EXPECT_EQ(1, invalid_out.get_received_messages().size());
  EXPECT_EQ(0, expired_out.get_received_messages().size());
  EXPECT_EQ(0, non_jwt_out.get_received_messages().size());
}

TEST(jwt_token, valid_jwt_token_validation_2) {
  std::string rsa_pub_key = R"(-----BEGIN PUBLIC KEY-----
MIIBIjANBgkqhkiG9w0BAQEFAAOCAQ8AMIIBCgKCAQEAxeqoZYbQ/Sr8DOFQ+/qb
EbCp6Vzb5hzH7oa3hf2FZxRKF0H6b8COMzz8+0mvEdYVvb/31jMEL2CIQhkQRol1
IruD6nBOmkjuXJSBficklMaJZORhuCrB4roHxzoG19aWmscA0gnfBKo2oGXSjJmn
ZxIh+2X6syHCfyMZZ00LzDyrgoXWQXyFvCA2ax54s7sKiHOM3P4A9W4QUwmoEi4H
QmPgJjIM4eGVPh0GtIANN+BOQ1KkUI7OzteHCTLu3VjxM0sw8QRayZdhniPF+U9n
3fa1mO4KLBsW4mDLjg8R/JuAGTX/SEEGj0B5HWQAP6myxKFz2xwDaCGvT+rdvkkt
OwIDAQAB
-----END PUBLIC KEY-----)";

  std::string token5 =
      "eyJ4NXQiOiJNell4TW1Ga09HWXdNV0kwWldObU5EY3hOR1l3WW1NNFpUQTNNV0kyTkRBelpHUXpOR00wWkdSbE5qSmtPREZrWkRSaU9URmtNV0ZoTXpVMlpHVmxOZyIsImtpZCI6Ik16WXhNbUZrT0dZd01XSTBaV05tTkRjeE5HWXdZbU00WlRBM01XSTJOREF6WkdRek5HTTBaR1JsTmpKa09ERmtaRFJpT1RGa01XRmhNelUyWkdWbE5nX1JTMjU2IiwiYWxnIjoiUlMyNTYifQ.eyJzdWIiOiJhZG1pbiIsImF1dCI6IkFQUExJQ0FUSU9OIiwiYXVkIjpbIlBzT01MRUdXc3I5bEJxNnFOcGI2VHI1Nm1oOGEiLCJhdWRfMSJdLCJuYmYiOjE2NDE4MTkwMDMsImF6cCI6IlBzT01MRUdXc3I5bEJxNnFOcGI2VHI1Nm1oOGEiLCJzY29wZSI6InJlYWRfYWNjZXNzIiwiaXNzIjoiaHR0cHM6XC9cL2xvY2FsaG9zdDo5NDQzXC9vYXV0aDJcL3Rva2VuIiwiZXhwIjoxNjczMzU1MDAzLCJpYXQiOjE2NDE4MTkwMDMsImp0aSI6IjY5MjM5MTNjLWI4NTctNDNhOS1iNjhiLTQ4NDE2OGQ5YjBiZCJ9.mSJeeL1eYKmbyPHxKHahWtKr789LcEw6K4m0fFDbOjNCa1j1jjI6lx4dlfQ1q7icTAe0FKw3BLpq31POZV9MdNyuQMUxpMKBdWhDf03Sk-UmK2CEl73qcLu8l6ao7deQ5RLl_IcCWBfrqF9IxO2rNs583SUSXYt3Ob2LA5nzKOcb6rmA-724zwxbvtIncOePJMdE6TRT68byb5m5bJgjL8wJHdcG_LHDDQFp-Ze3-MXzQ3yUpw4AkzfqiFWjYql8RXC1dh9KfMcOZ-HGRcTYc23M986AXCczkXRweZD3oYyaZP_CuhWiHHMGpKV8fQX5IWXJ0XJta3c6QNbUF9U5yw";

  stages::jwt_token::JwtValidationStage::field_combination_t token_field = {"http_info", "authorization"};

  stages::jwt_token::JwtValidationStage
      jwt_validate_stage{"valid_jwt_token_validation_2", token_field, "token_info", rsa_pub_key};

  event_processing::DynamicMessage dynamic_message_5;

  MessageCollectorSink valid_out{"valid_out"};
  MessageCollectorSink invalid_out{"invalid_out"};
  MessageCollectorSink expired_out{"expired_out"};
  MessageCollectorSink non_jwt_out{"non_jwt_out"};

  jwt_validate_stage.get_source("valid_out")->set_sink(&valid_out);
  jwt_validate_stage.get_source("invalid_out")->set_sink(&invalid_out);
  jwt_validate_stage.get_source("expired_out")->set_sink(&expired_out);
  jwt_validate_stage.get_source("non_jwt_out")->set_sink(&non_jwt_out);

  dynamic_message_5.set_field<std::string>("http_info", "authorization", token5);

  EXPECT_NO_THROW(jwt_validate_stage.get_sink("message_in")->on_message(&dynamic_message_5));

  EXPECT_EQ(1, valid_out.get_received_messages().size());
  EXPECT_EQ(0, invalid_out.get_received_messages().size());
  EXPECT_EQ(0, expired_out.get_received_messages().size());
  EXPECT_EQ(0, non_jwt_out.get_received_messages().size());
}

TEST(jwt_token, valid_jwt_token_validation_3) {
  std::string rsa_pub_key = R"(-----BEGIN PUBLIC KEY-----
MIIBIjANBgkqhkiG9w0BAQEFAAOCAQ8AMIIBCgKCAQEAxeqoZYbQ/Sr8DOFQ+/qb
EbCp6Vzb5hzH7oa3hf2FZxRKF0H6b8COMzz8+0mvEdYVvb/31jMEL2CIQhkQRol1
IruD6nBOmkjuXJSBficklMaJZORhuCrB4roHxzoG19aWmscA0gnfBKo2oGXSjJmn
ZxIh+2X6syHCfyMZZ00LzDyrgoXWQXyFvCA2ax54s7sKiHOM3P4A9W4QUwmoEi4H
QmPgJjIM4eGVPh0GtIANN+BOQ1KkUI7OzteHCTLu3VjxM0sw8QRayZdhniPF+U9n
3fa1mO4KLBsW4mDLjg8R/JuAGTX/SEEGj0B5HWQAP6myxKFz2xwDaCGvT+rdvkkt
OwIDAQAB
-----END PUBLIC KEY-----)";

  std::string token6 =
      "eyJ4NXQiOiJNell4TW1Ga09HWXdNV0kwWldObU5EY3hOR1l3WW1NNFpUQTNNV0kyTkRBelpHUXpOR00wWkdSbE5qSmtPREZrWkRSaU9URmtNV0ZoTXpVMlpHVmxOZyIsImtpZCI6Ik16WXhNbUZrT0dZd01XSTBaV05tTkRjeE5HWXdZbU00WlRBM01XSTJOREF6WkdRek5HTTBaR1JsTmpKa09ERmtaRFJpT1RGa01XRmhNelUyWkdWbE5nX1JTMjU2IiwiYWxnIjoiUlMyNTYifQ.eyJzdWIiOiJhZG1pbiIsImF1dCI6IkFQUExJQ0FUSU9OIiwiYXVkIjpbIkM3ZFV4b0VaZ0w4Z1g4WjZ6YUNTdzM3M1ZLOGEiLCJhdWRfMyIsImF1ZF8yIl0sIm5iZiI6MTY0MTgxOTE3NiwiYXpwIjoiQzdkVXhvRVpnTDhnWDhaNnphQ1N3MzczVks4YSIsInNjb3BlIjoicmVhZF9hY2Nlc3Mgd3JpdGVfYWNjZXNzIiwiaXNzIjoiaHR0cHM6XC9cL2xvY2FsaG9zdDo5NDQzXC9vYXV0aDJcL3Rva2VuIiwiZXhwIjoxNjczMzU1MTc2LCJpYXQiOjE2NDE4MTkxNzYsImp0aSI6ImQzNzZiOGEzLWQ0ODUtNDFmZS1iZjY5LTI4NmYzZjRhMWZmZCJ9.rPBgpxx_Ik2sm-fm1_k_KQMeIKix7Tls-NUP5Ze2C_qsRLPTBYQ_udInbXplBonASGvSHCHZ4qQJA3bDtYnyoUYY_wcJ-aQQVjCl_WL27ZBDeT9ltsQbDAABXpUXcY2dwrQiOXz0ANyBTrDy5H1RV_9WYr5-PPCtBYbnNCHl6TDbLCDrjoEJ7GKso6voQGo5uWmkK3NA06J1GrNwK6SmrKwV53mr7l1RkbQsQLffE-AIK_2PmrP1srrlNlFdWWTC4KxgMJnwDWFFBaGD3V_LJ-auNeywmGYCLVg_9Omp4Pop3bLXdO0Ia99e0Smq1OAmJi8HomLdN2MdyHtzxui1uw";

  stages::jwt_token::JwtValidationStage::field_combination_t token_field = {"http_info", "authorization"};

  stages::jwt_token::JwtValidationStage
      jwt_validate_stage{"valid_jwt_token_validation_3", token_field, "token_info", rsa_pub_key};

  event_processing::DynamicMessage dynamic_message_6;

  MessageCollectorSink valid_out{"valid_out"};
  MessageCollectorSink invalid_out{"invalid_out"};
  MessageCollectorSink expired_out{"expired_out"};
  MessageCollectorSink non_jwt_out{"non_jwt_out"};

  jwt_validate_stage.get_source("valid_out")->set_sink(&valid_out);
  jwt_validate_stage.get_source("invalid_out")->set_sink(&invalid_out);
  jwt_validate_stage.get_source("expired_out")->set_sink(&expired_out);
  jwt_validate_stage.get_source("non_jwt_out")->set_sink(&non_jwt_out);

  dynamic_message_6.set_field<std::string>("http_info", "authorization", token6);

  EXPECT_NO_THROW(jwt_validate_stage.get_sink("message_in")->on_message(&dynamic_message_6));

  EXPECT_EQ(1, valid_out.get_received_messages().size());
  EXPECT_EQ(0, invalid_out.get_received_messages().size());
  EXPECT_EQ(0, expired_out.get_received_messages().size());
  EXPECT_EQ(0, non_jwt_out.get_received_messages().size());
}

}
