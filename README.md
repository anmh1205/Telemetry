# Các bước sử dụng:

1. Dùng PlatformIO trên VS Code, build project trong folder Node và nạp cho ESP32
2. Khai báo thư viện.
3. Tạo struct TELEMETRY_S.
4. Gọi TM_Init(): Gắn struct và khai báo bộ UART giao tiếp với ESP32.
5. Gọi TM_SetNodeName(): Đặt tên cho node để theo dõi trên giao diện.
6. Gọi TM_SetIdAssign(): Tạo các trường thông tin cần theo dõi, và gắn ID cho trường thông tin đó.
7. Gọi TM_SetDataField() để cập nhật giá trị cho trường thông tin, sau đó gọi TM_PublishData() để chốt gửi thông tin.

# Chú ý:

1. Mỗi lần gọi TM_PublishData() sẽ gửi thông tin lên server 1 lần, vậy nên hãy cập nhật nhiều trường thông tin, rồi mới gọi TM_PublishData() để giảm tải server.
2. Đọc kĩ các bước sử dụng trong file README này, và đọc file code example để nắm rõ cách sử dụng
