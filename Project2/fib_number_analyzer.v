module fib_number_analyzer(in_number, clock, reset, enable, out_ready, is_fib);
input clock;
input reset;
input enable;
input[31:0] in_number; 
output reg out_ready;
output reg is_fib;

parameter  
	S0=3'b000, // Initial/Reset State
	S1=3'b001, // Condition check State
	S2=3'b011, // Fibonacci Increment State
	S3=3'b010, // Fibonacci state
	S4=3'b110; // Non-Fibonacci state
reg [2:0] current_state = S0, next_state;
reg[31:0] fib_0;
reg[31:0] fib_1;
reg[31:0] fib_temp;
reg[31:0] num;
always @(posedge clock, posedge reset)
begin
	if(reset)
		current_state <= S0;
	else if (enable == 0)
		current_state <= S0;
	else
		current_state <= next_state;
end

// Next state logic
always @(current_state, num, fib_temp)
begin
	case(current_state) 
		S0: begin
			next_state <= S1;
		end
		S1: begin
			if($signed(fib_temp) == $signed(num))
				next_state <= S3; 
			else if ($signed(fib_temp) < $signed(num))
				next_state <= S2;
			else
				next_state <= S4;
		end
		S2: begin // Increment STATE
			next_state <= S1;
		end
		S3: begin // Fibonacci STATE
			next_state <= current_state;
		end
		S4: begin // Non-Fibonacci STATE
			next_state <= current_state;
		end
		default:
			next_state <= S0;
	endcase
end

always @(current_state)
begin 
	case(current_state)
		S0: begin
			fib_0 <= 0;
			fib_1 <= 1;
			fib_temp <= 0;
			is_fib <= 0;
			out_ready <= 0;
			num <= in_number;
		end
		S2: begin
			fib_temp <= fib_0 + fib_1;
			fib_0 <= fib_1;
			fib_1 <= fib_0 + fib_1;
		end
		S3: begin 
			out_ready <= 1;
			is_fib <= 1;
		end
		S4: begin 
			out_ready <= 1;
			is_fib <= 0;
		end
		default: begin
			out_ready <= 0;
			is_fib <=0;
		end
	endcase
end 

endmodule