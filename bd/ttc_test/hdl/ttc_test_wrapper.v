module ttc_test_wrapper (
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

  wire DDR_ras_n;
  wire DDR_cas_n;
  wire DDR_cke;
  wire DDR_ck_n;
  wire DDR_ck_p;
  wire DDR_cs_n;
  wire DDR_reset_n;
  wire DDR_odt;
  wire DDR_we_n;
  wire [2:0]DDR_ba;
  wire [14:0]DDR_addr;
  wire [3:0]DDR_dm;
  wire [31:0]DDR_dq;
  wire [3:0]DDR_dqs_n;
  wire [3:0]DDR_dqs_p;
  wire [53:0]FIXED_IO_mio;
  wire FIXED_IO_ps_porb;
  wire FIXED_IO_ddr_vrn;
  wire FIXED_IO_ps_srstb;
  wire FIXED_IO_ddr_vrp;
  wire FIXED_IO_ps_clk;

  ttc_test ttc_test_i (
        .DDR_ras_n(DDR_ras_n),
        .DDR_cas_n(DDR_cas_n),
        .DDR_cke(DDR_cke),
        .DDR_ck_n(DDR_ck_n),
        .DDR_ck_p(DDR_ck_p),
        .DDR_cs_n(DDR_cs_n),
        .DDR_reset_n(DDR_reset_n),
        .DDR_odt(DDR_odt),
        .DDR_we_n(DDR_we_n),
        .DDR_ba(DDR_ba),
        .DDR_addr(DDR_addr),
        .DDR_dm(DDR_dm),
        .DDR_dq(DDR_dq),
        .DDR_dqs_n(DDR_dqs_n),
        .DDR_dqs_p(DDR_dqs_p),
        .FIXED_IO_mio(FIXED_IO_mio),
        .FIXED_IO_ps_porb(FIXED_IO_ps_porb),
        .FIXED_IO_ddr_vrn(FIXED_IO_ddr_vrn),
        .FIXED_IO_ps_srstb(FIXED_IO_ps_srstb),
        .FIXED_IO_ddr_vrp(FIXED_IO_ddr_vrp),
        .FIXED_IO_ps_clk(FIXED_IO_ps_clk));
endmodule
