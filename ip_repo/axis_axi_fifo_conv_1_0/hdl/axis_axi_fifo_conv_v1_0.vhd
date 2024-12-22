library ieee;
  use ieee.std_logic_1164.all;
  use ieee.numeric_std.all;

entity axis_axi_fifo_conv_v1_0 is
  generic (
    -- Users to add parameters here
    -- User parameters ends
    -- Do not modify the parameters beyond this line
    -- Parameters of Axi Slave Bus Interface S00_AXI
    C_S00_AXI_DATA_WIDTH   : integer := 32;
    C_S00_AXI_ADDR_WIDTH   : integer := 4;

    -- Parameters of Axi Slave Bus Interface SDR_AXIS
    C_SDR_AXIS_TDATA_WIDTH : integer := 32
  );
  port (
    -- Users to add ports here
    -- User ports ends
    -- Do not modify the ports beyond this line
    -- Ports of Axi Slave Bus Interface S00_AXI
    s00_axi_aclk     : in  std_logic;
    s00_axi_aresetn  : in  std_logic;
    s00_axi_awaddr   : in  std_logic_vector(C_S00_AXI_ADDR_WIDTH - 1 downto 0);
    s00_axi_awprot   : in  std_logic_vector(2 downto 0);
    s00_axi_awvalid  : in  std_logic;
    s00_axi_awready  : out std_logic;
    s00_axi_wdata    : in  std_logic_vector(C_S00_AXI_DATA_WIDTH - 1 downto 0);
    s00_axi_wstrb    : in  std_logic_vector((C_S00_AXI_DATA_WIDTH / 8) - 1 downto 0);
    s00_axi_wvalid   : in  std_logic;
    s00_axi_wready   : out std_logic;
    s00_axi_bresp    : out std_logic_vector(1 downto 0);
    s00_axi_bvalid   : out std_logic;
    s00_axi_bready   : in  std_logic;
    s00_axi_araddr   : in  std_logic_vector(C_S00_AXI_ADDR_WIDTH - 1 downto 0);
    s00_axi_arprot   : in  std_logic_vector(2 downto 0);
    s00_axi_arvalid  : in  std_logic;
    s00_axi_arready  : out std_logic;
    s00_axi_rdata    : out std_logic_vector(C_S00_AXI_DATA_WIDTH - 1 downto 0);
    s00_axi_rresp    : out std_logic_vector(1 downto 0);
    s00_axi_rvalid   : out std_logic;
    s00_axi_rready   : in  std_logic;

    -- Ports of Axi Slave Bus Interface SDR_AXIS
    sdr_axis_aclk    : in  std_logic;
    sdr_axis_aresetn : in  std_logic;
    sdr_axis_tready  : out std_logic;
    sdr_axis_tdata   : in  std_logic_vector(C_SDR_AXIS_TDATA_WIDTH - 1 downto 0);
    sdr_axis_tstrb   : in  std_logic_vector((C_SDR_AXIS_TDATA_WIDTH / 8) - 1 downto 0);
    sdr_axis_tlast   : in  std_logic;
    sdr_axis_tvalid  : in  std_logic
  );
end entity;

architecture arch_imp of axis_axi_fifo_conv_v1_0 is

  -- component declaration
  component axis_axi_fifo_conv_v1_0_S00_AXI is
    generic (
      C_S_AXI_DATA_WIDTH : integer := 32;
      C_S_AXI_ADDR_WIDTH : integer := 4
    );
    port (
      reg_tready        : out std_logic;
      reg_tvalid        : in  std_logic;
      reg_tdata         : in  std_logic_vector(C_S_AXI_DATA_WIDTH - 1 downto 0);
      reg_rd_cnt        : in  std_logic_vector(C_S_AXI_DATA_WIDTH - 1 downto 0);
      S_AXI_ACLK        : in  std_logic;
      S_AXI_ARESETN     : in  std_logic;
      S_AXI_AWADDR      : in  std_logic_vector(C_S_AXI_ADDR_WIDTH - 1 downto 0);
      S_AXI_AWPROT      : in  std_logic_vector(2 downto 0);
      S_AXI_AWVALID     : in  std_logic;
      S_AXI_AWREADY     : out std_logic;
      S_AXI_WDATA       : in  std_logic_vector(C_S_AXI_DATA_WIDTH - 1 downto 0);
      S_AXI_WSTRB       : in  std_logic_vector((C_S_AXI_DATA_WIDTH / 8) - 1 downto 0);
      S_AXI_WVALID      : in  std_logic;
      S_AXI_WREADY      : out std_logic;
      S_AXI_BRESP       : out std_logic_vector(1 downto 0);
      S_AXI_BVALID      : out std_logic;
      S_AXI_BREADY      : in  std_logic;
      S_AXI_ARADDR      : in  std_logic_vector(C_S_AXI_ADDR_WIDTH - 1 downto 0);
      S_AXI_ARPROT      : in  std_logic_vector(2 downto 0);
      S_AXI_ARVALID     : in  std_logic;
      S_AXI_ARREADY     : out std_logic;
      S_AXI_RDATA       : out std_logic_vector(C_S_AXI_DATA_WIDTH - 1 downto 0);
      S_AXI_RRESP       : out std_logic_vector(1 downto 0);
      S_AXI_RVALID      : out std_logic;
      S_AXI_RREADY      : in  std_logic
    );
  end component;

  component axis_data_fifo_0
    port (
      s_axis_aresetn     : in  STD_LOGIC;
      s_axis_aclk        : in  STD_LOGIC;
      s_axis_tvalid      : in  STD_LOGIC;
      s_axis_tready      : out STD_LOGIC;
      s_axis_tdata       : in  STD_LOGIC_VECTOR(31 downto 0);
      m_axis_aclk        : in  STD_LOGIC;
      m_axis_tvalid      : out STD_LOGIC;
      m_axis_tready      : in  STD_LOGIC;
      m_axis_tdata       : out STD_LOGIC_VECTOR(31 downto 0);
      axis_wr_data_count : out STD_LOGIC_VECTOR(31 downto 0);
      axis_rd_data_count : out STD_LOGIC_VECTOR(31 downto 0)
    );
  end component;

  -- user signals 
  signal s00_axi_rvalid_sig : std_logic;
  signal reg_valid_sig  : std_logic;
  signal reg_ready_sig  : std_logic;
  signal reg_data_sig   : std_logic_vector(C_SDR_AXIS_TDATA_WIDTH - 1 downto 0);
  signal reg_rd_cnt_sig : std_logic_vector(C_SDR_AXIS_TDATA_WIDTH - 1 downto 0);
  signal reg_wr_cnt_sig : std_logic_vector(C_SDR_AXIS_TDATA_WIDTH - 1 downto 0);
  signal slv_reg_rden_port : std_logic;

begin

  -- get data in from SDR, send to S00 AXI BUS
  i_axis_data_fifo_0: axis_data_fifo_0
    port map (
      s_axis_aresetn     => sdr_axis_aresetn,
      s_axis_aclk        => sdr_axis_aclk,
      s_axis_tvalid      => sdr_axis_tvalid,
      s_axis_tready      => sdr_axis_tready,
      s_axis_tdata       => sdr_axis_tdata,
      m_axis_aclk        => s00_axi_aclk,
      m_axis_tvalid      => reg_valid_sig,
      m_axis_tready      => reg_ready_sig,
      m_axis_tdata       => reg_data_sig,
      axis_wr_data_count => reg_wr_cnt_sig,
      axis_rd_data_count => reg_rd_cnt_sig
    );

  -- Instantiation of Axi Bus Interface S00_AXI
  axis_axi_fifo_conv_v1_0_S00_AXI_inst: axis_axi_fifo_conv_v1_0_S00_AXI
    generic map (
      C_S_AXI_DATA_WIDTH => C_S00_AXI_DATA_WIDTH,
      C_S_AXI_ADDR_WIDTH => C_S00_AXI_ADDR_WIDTH
    )
    port map (
      reg_tready        => reg_ready_sig,
      reg_tvalid        => reg_valid_sig,
      reg_tdata         => reg_data_sig,
      reg_rd_cnt        => reg_rd_cnt_sig,
      S_AXI_ACLK        => s00_axi_aclk,
      S_AXI_ARESETN     => s00_axi_aresetn,
      S_AXI_AWADDR      => s00_axi_awaddr,
      S_AXI_AWPROT      => s00_axi_awprot,
      S_AXI_AWVALID     => s00_axi_awvalid,
      S_AXI_AWREADY     => s00_axi_awready,
      S_AXI_WDATA       => s00_axi_wdata,
      S_AXI_WSTRB       => s00_axi_wstrb,
      S_AXI_WVALID      => s00_axi_wvalid,
      S_AXI_WREADY      => s00_axi_wready,
      S_AXI_BRESP       => s00_axi_bresp,
      S_AXI_BVALID      => s00_axi_bvalid,
      S_AXI_BREADY      => s00_axi_bready,
      S_AXI_ARADDR      => s00_axi_araddr,
      S_AXI_ARPROT      => s00_axi_arprot,
      S_AXI_ARVALID     => s00_axi_arvalid,
      S_AXI_ARREADY     => s00_axi_arready,
      S_AXI_RDATA       => s00_axi_rdata,
      S_AXI_RRESP       => s00_axi_rresp,
      S_AXI_RVALID      => s00_axi_rvalid_sig,
      S_AXI_RREADY      => s00_axi_rready
    );

  -- Add user logic here
  s00_axi_rvalid <= s00_axi_rvalid_sig;
  -- User logic ends
end architecture;
