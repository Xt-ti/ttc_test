module ttc_test (
    DDR_ras_n,
    DDR_cas_n,
    DDR_cke,
    DDR_ck_n,
    DDR_ck_p,
    DDR_cs_n,
    DDR_reset_n,
    DDR_odt,
    DDR_we_n,
    DDR_ba,
    DDR_addr,
    DDR_dm,
    DDR_dq,
    DDR_dqs_n,
    DDR_dqs_p,
    FIXED_IO_mio,
    FIXED_IO_ps_porb,
    FIXED_IO_ddr_vrn,
    FIXED_IO_ps_srstb,
    FIXED_IO_ddr_vrp,
    FIXED_IO_ps_clk);

  inout DDR_ras_n;
  inout DDR_cas_n;
  inout DDR_cke;
  inout DDR_ck_n;
  inout DDR_ck_p;
  inout DDR_cs_n;
  inout DDR_reset_n;
  inout DDR_odt;
  output DDR_we_n;
  inout [2:0]DDR_ba;
  inout [14:0]DDR_addr;
  inout [3:0]DDR_dm;
  inout [31:0]DDR_dq;
  inout [3:0]DDR_dqs_n;
  inout [3:0]DDR_dqs_p;
  inout [53:0]FIXED_IO_mio;
  inout FIXED_IO_ps_porb;
  inout FIXED_IO_ddr_vrn;
  inout FIXED_IO_ps_srstb;
  inout FIXED_IO_ddr_vrp;
  inout FIXED_IO_ps_clk;

  wire [14:0]processing_system7_0_DDR_Addr;
  wire [2:0]processing_system7_0_DDR_BankAddr;
  wire processing_system7_0_DDR_CAS_n;
  wire processing_system7_0_DDR_CKE;
  wire processing_system7_0_DDR_CS_n;
  wire processing_system7_0_DDR_Clk;
  wire processing_system7_0_DDR_Clk_n;
  wire [3:0]processing_system7_0_DDR_DM;
  wire [31:0]processing_system7_0_DDR_DQ;
  wire [3:0]processing_system7_0_DDR_DQS;
  wire [3:0]processing_system7_0_DDR_DQS_n;
  wire processing_system7_0_DDR_DRSTB;
  wire processing_system7_0_DDR_ODT;
  wire processing_system7_0_DDR_RAS_n;
  wire processing_system7_0_DDR_VRN;
  wire processing_system7_0_DDR_VRP;
  wire processing_system7_0_DDR_WEB;
  wire [53:0]processing_system7_0_MIO;
  wire processing_system7_0_PS_CLK;
  wire processing_system7_0_PS_PORB;
  wire processing_system7_0_PS_SRSTB;

  assign DDR_we_n = processing_system7_0_DDR_WEB;
  ttc_test_processing_system7_0 processing_system7_0 (
        .TTC0_WAVE0_OUT(),
        .TTC0_WAVE1_OUT(),
        .TTC0_WAVE2_OUT(),
        .MIO(FIXED_IO_mio),
        .DDR_Clk(DDR_ck_p),
        .DDR_Clk_n(DDR_ck_n),
        .DDR_CKE(DDR_cke),
        .DDR_CS_n(DDR_cs_n),
        .DDR_RAS_n(DDR_ras_n),
        .DDR_CAS_n(DDR_cas_n),
        .DDR_WEB(processing_system7_0_DDR_WEB),
        .DDR_BankAddr(DDR_ba),
        .DDR_Addr(DDR_addr),
        .DDR_ODT(DDR_odt),
        .DDR_DRSTB(DDR_reset_n),
        .DDR_DQ(DDR_dq),
        .DDR_DM(DDR_dm),
        .DDR_DQS(DDR_dqs_p),
        .DDR_DQS_n(DDR_dqs_n),
        .DDR_VRN(FIXED_IO_ddr_vrn),
        .DDR_VRP(FIXED_IO_ddr_vrp),
        .PS_SRSTB(FIXED_IO_ps_srstb),
        .PS_CLK(FIXED_IO_ps_clk),
        .PS_PORB(FIXED_IO_ps_porb));
endmodule

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

