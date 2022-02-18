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
 *   @author Braveenan Sritharan (braveenan.sritharan@axiatadigitallabs.com) on 11/17/21.
 *
 */

#include <gtest/gtest.h>
#include "../../stages/http_server_stage/http_server_stage.h"

namespace adl::axp::core::stages::http_server_stage::test {

TEST(query_parameter, extration_1) {
  std::string_view query_test = "q1=5&q2=7";

  EXPECT_EQ(extract_query(query_test)["q1"] == "5", true);
  EXPECT_EQ(extract_query(query_test)["q2"] == "7", true);
}

TEST(query_parameter, extration_2) {
  std::string_view query_test = "q4=ty&q6=yu";

  EXPECT_EQ(extract_query(query_test)["q4"] == "ty", true);
  EXPECT_EQ(extract_query(query_test)["q6"] == "yu", true);
}

TEST(query_parameter, extration_3) {
  std::string_view query_test = "q12=4.5&q26=7.8";

  EXPECT_EQ(extract_query(query_test)["q12"] == "4.5", true);
  EXPECT_EQ(extract_query(query_test)["q26"] == "7.8", true);
}

TEST(query_parameter, extration_4) {
  std::string_view query_test = "q125=4.5678&q6=678";

  EXPECT_EQ(extract_query(query_test)["q125"] == "4.5678", true);
  EXPECT_EQ(extract_query(query_test)["q6"] == "678", true);
}

TEST(query_parameter, extration_5) {
  std::string_view query_test = "q1254.5678&q6678";

  EXPECT_EQ(extract_query(query_test)["q125"] == "4.5678", false);
  EXPECT_EQ(extract_query(query_test)["q6"] == "678", false);
}

TEST(query_parameter, extration_6) {
  std::string_view query_test = "q125=4.5678q6=678";

  EXPECT_EQ(extract_query(query_test)["q125"] == "4.5678", false);
  EXPECT_EQ(extract_query(query_test)["q6"] == "678", false);
}

TEST(query_parameter, extration_7) {
  std::string_view query_test = "q1254.5678q6678";

  EXPECT_EQ(extract_query(query_test)["q125"] == "4.5678", false);
  EXPECT_EQ(extract_query(query_test)["q6"] == "678", false);
}

TEST(query_parameter, extration_8) {
  std::string_view query_test = "q1=t?%265&q2=7?%268";

  EXPECT_EQ(extract_query(query_test)["q1"] == "t?%265", true);
  EXPECT_EQ(extract_query(query_test)["q2"] == "7?%268", true);
}

TEST(query_parameter, extration_9) {
  std::string_view query_test = "%26=78&%3D=r";

  EXPECT_EQ(extract_query(query_test)["%26"] == "78", true);
  EXPECT_EQ(extract_query(query_test)["%3D"] == "r", true);
}

TEST(query_parameter, extration_10) {
  std::string_view query_test = "?=u7&8=%26&y==";

  EXPECT_EQ(extract_query(query_test)["y"] == "=", true);
  EXPECT_EQ(extract_query(query_test)["8"] == "%26", true);
  EXPECT_EQ(extract_query(query_test)["?"] == "u7", true);
}
TEST(http_validations, path_parameter_extraction) {
  const auto resource_path1 = "test_1/:path_param_0/mock/:path_param_1";
  const auto resource_path1_path_params = extract_path_parameters(resource_path1);

  EXPECT_EQ(resource_path1_path_params[0] == "path_param_0", true);
  EXPECT_EQ(resource_path1_path_params[1] == "path_param_1", true);

  const auto resource_path2 = "test_1/:p0/mock/:p1/:p2/:p4/:p3/:p10";
  const auto resource_path2_path_params = extract_path_parameters(resource_path2);

  EXPECT_EQ(resource_path2_path_params[0] == "p0", true);
  EXPECT_EQ(resource_path2_path_params[1] == "p1", true);
  EXPECT_EQ(resource_path2_path_params[2] == "p2", true);
  EXPECT_EQ(resource_path2_path_params[3] == "p4", true);
  EXPECT_EQ(resource_path2_path_params[4] == "p3", true);
  EXPECT_EQ(resource_path2_path_params[5] == "p10", true);

  const auto resource_path3 = "/";
  const auto resource_path3_path_params = extract_path_parameters(resource_path3);

  EXPECT_EQ(resource_path3_path_params.empty() == true, true);

}

}