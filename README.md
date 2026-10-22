# HỆ THỐNG QUẢN LÝ DỊCH VỤ FTTH (BTL KTLT2)

Dự án Quản lý Dịch vụ Viễn thông FTTH - Bài tập lớn môn Kỹ thuật Lập trình 2.

---

## 1. Cấu Trúc Thư Mục (Codebase Architecture)

Mã nguồn dự án được tổ chức lại theo mô hình chuẩn, rõ ràng cho từng thành phần:

```text
btl-ktlt2/
├── bin/                          # Chứa các file thực thi sau khi biên dịch
│   └── main.exe                  # File thực thi Windows đã biên dịch sẵn
├── data/                         # Thư mục lưu trữ cơ sở dữ liệu dạng text (.txt)
│   ├── README.txt                # Hướng dẫn về dữ liệu các file
│   ├── customers.txt             # Dữ liệu khách hàng
│   ├── registrations.txt         # Dữ liệu đăng ký mới
│   ├── plans.txt                 # Dữ liệu các gói cước
│   ├── plan_changes.txt          # Dữ liệu thay đổi / chuyển gói cước
│   ├── contracts.txt             # Dữ liệu hợp đồng
│   ├── renewals.txt              # Dữ liệu gia hạn dịch vụ
│   ├── payments.txt              # Dữ liệu thanh toán
│   ├── invoices.txt              # Dữ liệu hóa đơn cước
│   ├── incidents.txt             # Dữ liệu báo cáo sự cố
│   └── supports.txt              # Dữ liệu phiếu hỗ trợ kỹ thuật
├── docs/                         # Tài liệu dự án
│   └── README_PHAN_CHIA_FILE.txt # Tài liệu mô tả phân chia file ban đầu
├── include/                      # Các file định nghĩa & khai báo (.h)
│   ├── AppData.h                 # Cấu trúc AppData tập trung các Repository
│   ├── Entities.h                # Lớp cơ sở Entity và 10 lớp thực thể
│   ├── InputHelper.h             # Tiện ích nhập liệu, định dạng console
│   ├── Menus.h                   # Khai báo các menu điều phối use case
│   ├── Repository.h              # Template Repository đọc/ghi file và quản lý danh sách
│   └── Validator.h               # Tiện ích kiểm tra tính hợp lệ dữ liệu (regex, ngày tháng)
├── src/                          # Mã nguồn chương trình (.cpp)
│   ├── main.cpp                  # Điểm khởi đầu chương trình (hàm main)
│   ├── Application.cpp           # Điều phối menu chính của 10 use case
│   └── use_cases/                # Hiện thực chi tiết 10 use case nghiệp vụ
│       ├── UC1_Customers.cpp     # Quản lý khách hàng
│       ├── UC2_Registrations.cpp # Đăng ký dịch vụ Internet
│       ├── UC3_Plans.cpp         # Quản lý gói cước
│       ├── UC4_PlanChanges.cpp   # Đăng ký / chuyển đổi gói cước
│       ├── UC5_Contracts.cpp     # Quản lý hợp đồng
│       ├── UC6_Renewals.cpp      # Gia hạn dịch vụ
│       ├── UC7_Payments.cpp      # Quản lý thanh toán
│       ├── UC8_Invoices.cpp      # Quản lý hóa đơn cước
│       ├── UC9_Incidents.cpp     # Quản lý sự cố Internet
│       └── UC10_Supports.cpp     # Hỗ trợ kỹ thuật
├── .vscode/
│   └── tasks.json                # Cấu hình build task cho VS Code (Ctrl+Shift+B)
├── .gitignore                    # Bỏ qua các file tạm, object files, build artifacts
├── CMakeLists.txt                # Cấu hình biên dịch bằng CMake
├── Makefile                      # Cấu hình biên dịch bằng Make
└── README.md                     # Tài liệu hướng dẫn sử dụng dự án
```

---

## 2. Hướng Dẫn Biên Dịch & Chạy Chương Trình

> [!NOTE]
> Do chương trình đọc/ghi các file dữ liệu từ đường dẫn tương đối `data/*.txt`, bạn **luôn chạy chương trình từ thư mục gốc của repository**.

### Cách 1: Sử dụng `make` (Khuyên dùng trên Linux / macOS / MSYS2)

- Biên dịch chương trình:
  ```bash
  make
  ```
- Chạy chương trình:
  ```bash
  make run
  ```
- Dọn dẹp các file build:
  ```bash
  make clean
  ```

---

### Cách 2: Biên dịch trực tiếp bằng `g++` (C++17)

Từ thư mục gốc của dự án, chạy lệnh:

- **Linux / macOS**:
  ```bash
  g++ -std=c++17 -Wall -Wextra -Iinclude src/main.cpp src/Application.cpp src/use_cases/*.cpp -o bin/main
  ./bin/main
  ```

- **Windows (MinGW)**:
  ```cmd
  g++ -std=c++17 -Wall -Wextra -Iinclude src/main.cpp src/Application.cpp src/use_cases/*.cpp -o bin/main.exe
  bin\main.exe
  ```

---

### Cách 3: Sử dụng VS Code

1. Mở thư mục dự án trong VS Code.
2. Nhấn `Ctrl + Shift + B` để thực thi task `"Build FTTH project (C++17)"`.
3. Chương trình sẽ được biên dịch vào thư mục `bin/`.

---

### Cách 4: Sử dụng `CMake`

```bash
cmake -B build
cmake --build build
./bin/ftth_app
```

---

## 3. Ghi Chú Phát Triển

- Các đoạn mã nguồn bên trong các file `.cpp` và `.h` được giữ nguyên vẹn logic, chỉ cấu trúc lại vị trí thư mục và đường dẫn.
- Khi bổ sung logic nghiệp vụ hoặc sửa đổi use case, làm việc trực tiếp trong `src/use_cases/`.
- Khi bổ sung thực thể hoặc cấu trúc dữ liệu, chỉnh sửa trong `include/Entities.h` và `include/AppData.h`.
