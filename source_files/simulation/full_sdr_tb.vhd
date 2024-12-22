library IEEE;
  use IEEE.std_logic_1164.all;
  use ieee.numeric_std.all;

entity full_sdr_tb is --=====================================================
  port (
    -- clock and reset
    resetn : in  std_logic;
    clk125 : in  std_logic;
    leds   : out std_logic_vector(3 downto 0);
    sw     : in  std_logic_vector(3 downto 0)
  );
end entity; --==================================================================

architecture sim of full_sdr_tb is --=================================

  ------------------------------------------------------------------------------
  --                              COMPONENTS                                  --
  ------------------------------------------------------------------------------
  component full_sdr is
    port (
      resetn            : in    std_logic;
      clk125            : in    std_logic;
      DDR_addr          : inout std_logic_vector(14 downto 0);
      DDR_ba            : inout std_logic_vector(2 downto 0);
      DDR_cas_n         : inout std_logic;
      DDR_ck_n          : inout std_logic;
      DDR_ck_p          : inout std_logic;
      DDR_cke           : inout std_logic;
      DDR_cs_n          : inout std_logic;
      DDR_dm            : inout std_logic_vector(3 downto 0);
      DDR_dq            : inout std_logic_vector(31 downto 0);
      DDR_dqs_n         : inout std_logic_vector(3 downto 0);
      DDR_dqs_p         : inout std_logic_vector(3 downto 0);
      DDR_odt           : inout std_logic;
      DDR_ras_n         : inout std_logic;
      DDR_reset_n       : inout std_logic;
      DDR_we_n          : inout std_logic;
      FIXED_IO_ddr_vrn  : inout std_logic;
      FIXED_IO_ddr_vrp  : inout std_logic;
      FIXED_IO_mio      : inout std_logic_vector(53 downto 0);
      FIXED_IO_ps_clk   : inout std_logic;
      FIXED_IO_ps_porb  : inout std_logic;
      FIXED_IO_ps_srstb : inout std_logic;
      leds              : out   std_logic_vector(3 downto 0);
      sw                : in    std_logic_vector(3 downto 0);
      SSM2603_bclk      : out   std_logic;
      SSM2603_pbdat     : out   std_logic;
      SSM2603_pblrc     : out   std_logic;
      SSM2603_recdat    : in    std_logic;
      SSM2603_reclrc    : out   std_logic;
      SSM2603_sda       : inout std_logic;
      SSM2603_scl       : inout std_logic;
      SSM2603_mute      : out   std_logic;
      SSM2603_mclk      : out   std_logic
    );
  end component;

  ------------------------------------------------------------------------------
  --                                SIGNALS                                   --
  ------------------------------------------------------------------------------
  signal DDR_addr          : std_logic_vector(14 downto 0);
  signal DDR_ba            : std_logic_vector(2 downto 0);
  signal DDR_cas_n         : std_logic;
  signal DDR_ck_n          : std_logic;
  signal DDR_ck_p          : std_logic;
  signal DDR_cke           : std_logic;
  signal DDR_cs_n          : std_logic;
  signal DDR_dm            : std_logic_vector(3 downto 0);
  signal DDR_dq            : std_logic_vector(31 downto 0);
  signal DDR_dqs_n         : std_logic_vector(3 downto 0);
  signal DDR_dqs_p         : std_logic_vector(3 downto 0);
  signal DDR_odt           : std_logic;
  signal DDR_ras_n         : std_logic;
  signal DDR_reset_n       : std_logic;
  signal DDR_we_n          : std_logic;
  signal FIXED_IO_ddr_vrn  : std_logic;
  signal FIXED_IO_ddr_vrp  : std_logic;
  signal FIXED_IO_mio      : std_logic_vector(53 downto 0);
  signal FIXED_IO_ps_clk   : std_logic;
  signal FIXED_IO_ps_porb  : std_logic;
  signal FIXED_IO_ps_srstb : std_logic;
  signal SSM2603_bclk      : std_logic;
  signal SSM2603_pbdat     : std_logic;
  signal SSM2603_pblrc     : std_logic;
  signal SSM2603_recdat    : std_logic;
  signal SSM2603_reclrc    : std_logic;
  signal SSM2603_sda       : std_logic;
  signal SSM2603_scl       : std_logic;
  signal SSM2603_mute      : std_logic;
  signal SSM2603_mclk      : std_logic;

begin --==========================================================================

  ------------------------------------------------------------------------------
  --                        COMPONENT INSTATIATIONS                           --
  ------------------------------------------------------------------------------
  i_full_sdr : full_sdr
    port map (
      resetn            => resetn,
      clk125            => clk125,
      DDR_addr          => DDR_addr,
      DDR_ba            => DDR_ba,
      DDR_cas_n         => DDR_cas_n,
      DDR_ck_n          => DDR_ck_n,
      DDR_ck_p          => DDR_ck_p,
      DDR_cke           => DDR_cke,
      DDR_cs_n          => DDR_cs_n,
      DDR_dm            => DDR_dm,
      DDR_dq            => DDR_dq,
      DDR_dqs_n         => DDR_dqs_n,
      DDR_dqs_p         => DDR_dqs_p,
      DDR_odt           => DDR_odt,
      DDR_ras_n         => DDR_ras_n,
      DDR_reset_n       => DDR_reset_n,
      DDR_we_n          => DDR_we_n,
      FIXED_IO_ddr_vrn  => FIXED_IO_ddr_vrn,
      FIXED_IO_ddr_vrp  => FIXED_IO_ddr_vrp,
      FIXED_IO_mio      => FIXED_IO_mio,
      FIXED_IO_ps_clk   => FIXED_IO_ps_clk,
      FIXED_IO_ps_porb  => FIXED_IO_ps_porb,
      FIXED_IO_ps_srstb => FIXED_IO_ps_srstb,
      leds              => leds,
      sw                => sw,
      SSM2603_bclk      => SSM2603_bclk,
      SSM2603_pbdat     => SSM2603_pbdat,
      SSM2603_pblrc     => SSM2603_pblrc,
      SSM2603_recdat    => SSM2603_recdat,
      SSM2603_reclrc    => SSM2603_reclrc,
      SSM2603_sda       => SSM2603_sda,
      SSM2603_scl       => SSM2603_scl,
      SSM2603_mute      => SSM2603_mute,
      SSM2603_mclk      => SSM2603_mclk
    );

  ------------------------------------------------------------------------------
  --                                PROCESSES                                 --
  ------------------------------------------------------------------------------
  ------------------------------------------------------------------------------
  --                               ILA INSTANCES                              --
  ------------------------------------------------------------------------------
end architecture;
