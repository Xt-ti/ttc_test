(* FM_CORE_INFO = "fmsh.com.cn:ip:processing_system7:1.0" *)
module ttc_test_processing_system7_0 (
  TTC0_WAVE0_OUT,
  TTC0_WAVE1_OUT,
  TTC0_WAVE2_OUT,
  MIO,
  DDR_Clk,
  DDR_Clk_n,
  DDR_CKE,
  DDR_CS_n,
  DDR_RAS_n,
  DDR_CAS_n,
  DDR_WEB,
  DDR_BankAddr,
  DDR_Addr,
  DDR_ODT,
  DDR_DRSTB,
  DDR_DQ,
  DDR_DM,
  DDR_DQS,
  DDR_DQS_n,
  DDR_VRN,
  DDR_VRP,
  PS_SRSTB,
  PS_CLK,
  PS_PORB
);

  output TTC0_WAVE0_OUT;
  output TTC0_WAVE1_OUT;
  output TTC0_WAVE2_OUT;
  inout [53 : 0] MIO;
  inout DDR_Clk;
  inout DDR_Clk_n;
  inout DDR_CKE;
  inout DDR_CS_n;
  inout DDR_RAS_n;
  inout DDR_CAS_n;
  output DDR_WEB;
  inout [2 : 0] DDR_BankAddr;
  inout [14 : 0] DDR_Addr;
  inout DDR_ODT;
  inout DDR_DRSTB;
  inout [31 : 0] DDR_DQ;
  inout [3 : 0] DDR_DM;
  inout [3 : 0] DDR_DQS;
  inout [3 : 0] DDR_DQS_n;
  inout DDR_VRN;
  inout DDR_VRP;
  input PS_SRSTB;
  input PS_CLK;
  input PS_PORB;

endmodule

