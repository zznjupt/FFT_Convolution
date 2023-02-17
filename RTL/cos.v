`include "define.v"

module cos (
    input  wire                        on,
    input  wire       [10:0]           h,
    output reg signed [`BITS-1:0]      value
);

always @(*) begin // Quantization
    if(on == 1) begin
        case(h)
            2:          value = -2097152;
            4:          value = 0;
            8:          value = 1482910;
            16:         value = 1937515;
            32:         value = 2056855;
            64:         value = 2087053;
            128:        value = 2094625;
            default:    value = 0;
        endcase
    end else begin
        case(h)
            2:          value = -2097152;
            4:          value = 0;
            8:          value = 1482910;
            16:         value = 1937515;
            32:         value = 2056855;
            64:         value = 2087053;
            128:        value = 2094625;
            default:    value = 0;
        endcase
    end
end

endmodule
