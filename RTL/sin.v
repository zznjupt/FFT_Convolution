`include "define.v"

module sin (
    input  wire                        on,
    input  wire       [10:0]           h,
    output reg signed [`BITS-1:0]      value
);

always @(*) begin // Quantization fp128 2 int65 by cpp simulation
    if(on == 1) begin
        case(h)
            2:          value = 0;
            4:          value = -2097152;
            8:          value = -1482910;
            16:         value = -802545;
            32:         value = -409134;
            64:         value = -205556;
            128:        value = -102902;
            default:    value = 0;
        endcase
    end else begin
        case(h)
            2:          value = 0;
            4:          value = 209152;
            8:          value = 1482910;
            16:         value = 802545;
            32:         value = 409134;
            64:         value = 205556;
            128:        value = 102902;
            default:    value = 0;
        endcase
    end
end

endmodule
