uniform sampler2D texture;
uniform float progress;

void main()
{
    // Lấy màu gốc của pixel từ texture
    vec4 pixel = texture2D(texture, gl_TexCoord[0].xy);

    // Tính toán vệt sáng chéo (đường x + y)
    // Toạ độ từ 0.0 đến 1.0. Để đường chéo đẹp, ta dùng x + y (từ 0.0 đến 2.0)
    // Chia cho 2.0 để đưa về khoảng 0.0 đến 1.0
    float diagonal = (gl_TexCoord[0].x + gl_TexCoord[0].y) / 2.0;

    // Khoảng cách từ pixel hiện tại đến tâm của vệt sáng (đang dịch chuyển theo progress)
    float dist = abs(diagonal - progress);

    // Độ rộng của vệt sáng
    float width = 0.1;

    // Cường độ sáng giảm dần theo khoảng cách đến tâm vệt sáng
    float intensity = max(0.0, 1.0 - (dist / width));

    // Thêm ánh sáng trắng (intensity) vào màu pixel gốc, nhưng giữ nguyên Alpha
    // Điều này đảm bảo ánh sáng chỉ xuất hiện trên hình ảnh, không bị lem ra viền trong suốt
    vec4 result = pixel + vec4(intensity, intensity, intensity, 0.0) * pixel.a;

    gl_FragColor = result;
}
