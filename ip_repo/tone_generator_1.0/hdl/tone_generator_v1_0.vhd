library ieee;
use ieee.std_logic_1164.all;
use ieee.numeric_std.all;

entity tone_generator_v1_0 is
	generic (
		-- Users to add parameters here

		-- User parameters ends
		-- Do not modify the parameters beyond this line


		-- Parameters of Axi Slave Bus Interface S00_AXI
		C_S00_AXI_DATA_WIDTH	: integer	:= 32;
		C_S00_AXI_ADDR_WIDTH	: integer	:= 4
	);
	port (
		-- Users to add ports here
		sample_clk_48k	: in std_logic;
        gen_output	: out std_logic_vector(23 downto 0);
		-- User ports ends
		-- Do not modify the ports beyond this line


		-- Ports of Axi Slave Bus Interface S00_AXI
		s00_axi_aclk	: in std_logic;
		s00_axi_aresetn	: in std_logic;
		s00_axi_awaddr	: in std_logic_vector(C_S00_AXI_ADDR_WIDTH-1 downto 0);
		s00_axi_awprot	: in std_logic_vector(2 downto 0);
		s00_axi_awvalid	: in std_logic;
		s00_axi_awready	: out std_logic;
		s00_axi_wdata	: in std_logic_vector(C_S00_AXI_DATA_WIDTH-1 downto 0);
		s00_axi_wstrb	: in std_logic_vector((C_S00_AXI_DATA_WIDTH/8)-1 downto 0);
		s00_axi_wvalid	: in std_logic;
		s00_axi_wready	: out std_logic;
		s00_axi_bresp	: out std_logic_vector(1 downto 0);
		s00_axi_bvalid	: out std_logic;
		s00_axi_bready	: in std_logic;
		s00_axi_araddr	: in std_logic_vector(C_S00_AXI_ADDR_WIDTH-1 downto 0);
		s00_axi_arprot	: in std_logic_vector(2 downto 0);
		s00_axi_arvalid	: in std_logic;
		s00_axi_arready	: out std_logic;
		s00_axi_rdata	: out std_logic_vector(C_S00_AXI_DATA_WIDTH-1 downto 0);
		s00_axi_rresp	: out std_logic_vector(1 downto 0);
		s00_axi_rvalid	: out std_logic;
		s00_axi_rready	: in std_logic
	);
end tone_generator_v1_0;

architecture arch_imp of tone_generator_v1_0 is
    signal frequency  : std_logic_vector(31 downto 0); -- actually not a frequency but whatever
    signal amplitude  : std_logic_vector(31 downto 0);
    signal period     : std_logic_vector(31 downto 0);
    signal duty_cycle : std_logic_vector(31 downto 0);
    
    signal freq_counter : std_logic_vector(31 downto 0);
    signal positive : std_logic;
    signal duty_counter : std_logic_vector(31 downto 0);

	-- component declaration
	component tone_generator_v1_0_S00_AXI is
		generic (
		C_S_AXI_DATA_WIDTH	: integer	:= 32;
		C_S_AXI_ADDR_WIDTH	: integer	:= 4
		);
		port (
		S_AXI_ACLK	: in std_logic;
		S_AXI_ARESETN	: in std_logic;
		S_AXI_AWADDR	: in std_logic_vector(C_S_AXI_ADDR_WIDTH-1 downto 0);
		S_AXI_AWPROT	: in std_logic_vector(2 downto 0);
		S_AXI_AWVALID	: in std_logic;
		S_AXI_AWREADY	: out std_logic;
		S_AXI_WDATA	: in std_logic_vector(C_S_AXI_DATA_WIDTH-1 downto 0);
		S_AXI_WSTRB	: in std_logic_vector((C_S_AXI_DATA_WIDTH/8)-1 downto 0);
		S_AXI_WVALID	: in std_logic;
		S_AXI_WREADY	: out std_logic;
		S_AXI_BRESP	: out std_logic_vector(1 downto 0);
		S_AXI_BVALID	: out std_logic;
		S_AXI_BREADY	: in std_logic;
		S_AXI_ARADDR	: in std_logic_vector(C_S_AXI_ADDR_WIDTH-1 downto 0);
		S_AXI_ARPROT	: in std_logic_vector(2 downto 0);
		S_AXI_ARVALID	: in std_logic;
		S_AXI_ARREADY	: out std_logic;
		S_AXI_RDATA	: out std_logic_vector(C_S_AXI_DATA_WIDTH-1 downto 0);
		S_AXI_RRESP	: out std_logic_vector(1 downto 0);
		S_AXI_RVALID	: out std_logic;
		S_AXI_RREADY	: in std_logic;

		SAMPLE_CLK_48K	: in std_logic;
		GEN_OUTPUT      : out std_logic_vector(23 downto 0);
		FREQUENCY       : out std_logic_vector(31 downto 0);
        AMPLITUDE       : out std_logic_vector(31 downto 0);
        PERIOD          : out std_logic_vector(31 downto 0);
        DUTY_CYCLE      : out std_logic_vector(31 downto 0)
		);
	end component tone_generator_v1_0_S00_AXI;

begin

-- Instantiation of Axi Bus Interface S00_AXI
tone_generator_v1_0_S00_AXI_inst : tone_generator_v1_0_S00_AXI
	generic map (
		C_S_AXI_DATA_WIDTH	=> C_S00_AXI_DATA_WIDTH,
		C_S_AXI_ADDR_WIDTH	=> C_S00_AXI_ADDR_WIDTH
	)
	port map (
		S_AXI_ACLK	=> s00_axi_aclk,
		S_AXI_ARESETN	=> s00_axi_aresetn,
		S_AXI_AWADDR	=> s00_axi_awaddr,
		S_AXI_AWPROT	=> s00_axi_awprot,
		S_AXI_AWVALID	=> s00_axi_awvalid,
		S_AXI_AWREADY	=> s00_axi_awready,
		S_AXI_WDATA	=> s00_axi_wdata,
		S_AXI_WSTRB	=> s00_axi_wstrb,
		S_AXI_WVALID	=> s00_axi_wvalid,
		S_AXI_WREADY	=> s00_axi_wready,
		S_AXI_BRESP	=> s00_axi_bresp,
		S_AXI_BVALID	=> s00_axi_bvalid,
		S_AXI_BREADY	=> s00_axi_bready,
		S_AXI_ARADDR	=> s00_axi_araddr,
		S_AXI_ARPROT	=> s00_axi_arprot,
		S_AXI_ARVALID	=> s00_axi_arvalid,
		S_AXI_ARREADY	=> s00_axi_arready,
		S_AXI_RDATA	=> s00_axi_rdata,
		S_AXI_RRESP	=> s00_axi_rresp,
		S_AXI_RVALID	=> s00_axi_rvalid,
		S_AXI_RREADY	=> s00_axi_rready,
		SAMPLE_CLK_48K  => sample_clk_48k,
		GEN_OUTPUT      => gen_output,
		FREQUENCY       => frequency,
        AMPLITUDE       => amplitude,
        PERIOD          => period,
        DUTY_CYCLE      => duty_cycle
	);


	-- Add user logic here
    process (sample_clk_48k, frequency, amplitude, period, duty_cycle, freq_counter, positive, duty_counter)
        variable out_temp : std_logic_vector(31 downto 0);
    begin
      if rising_edge(sample_clk_48k) then
          if freq_counter >= frequency then
              freq_counter <= (others => '0');
              positive <= not positive;
              if duty_counter <= duty_cycle then
                  if positive = '1' then
                      out_temp := amplitude;
                  else
                      out_temp := std_logic_vector(0 - signed(amplitude));
                  end if;
              else
                  out_temp := (others => '0');
              end if;
              gen_output <= out_temp(31) & out_temp(22 downto 0);
          else
              freq_counter <= std_logic_vector(unsigned(freq_counter) + 1);
          end if;

          if duty_counter >= period then
              duty_counter <= (others => '0');
          else
              duty_counter <= std_logic_vector(unsigned(duty_counter) + 1);
          end if;
      end if;
    end process;
	-- User logic ends

end arch_imp;
