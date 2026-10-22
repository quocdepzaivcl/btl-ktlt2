# Codebase Audit & Problem Report

This document compiles all findings from the automated test suite, static code analysis, and architectural review of the FTTH Service Management codebase.

---

## 1. Compilation & Header Linking Status

- **Build Status**: **PASSED** (0 errors, 0 warnings with `-std=c++17 -Wall -Wextra`).
- **Header Resolution**: All header files in `include/` (`AppData.h`, `Entities.h`, `InputHelper.h`, `Menus.h`, `Repository.h`, `Validator.h`) resolve correctly across `src/` and `src/use_cases/` via the `-Iinclude` compiler flag configured in both `Makefile` and `CMakeLists.txt`.
- **Automated Test Suite**: An automated test suite ([tests/test_runner.cpp](file:///home/tungle2202/ngu/btl-ktlt2/tests/test_runner.cpp)) was created, running **195 assertions** covering validators, tokenizers, entity round-trips, repository CRUD operations, and referential integrity. All 195 unit & integration assertions pass successfully.

---

## 2. Data Anomalies & Initial State Inconsistencies

### 2.1. Corrupt Record in `data/customers.txt`
- **Location**: [data/customers.txt](file:///home/tungle2202/ngu/btl-ktlt2/data/customers.txt) (line 11)
- **Content**: `213|4235|5467|4657|4657|`
- **Severity**: **Medium**
- **Description**: 
  - Customer ID is `213` (violates `KHxxx` format convention).
  - Phone number is `5467` (violates 10-digit Vietnamese phone standard).
  - National ID (CCCD) is `4657` (violates 12-digit standard).
  - Address is `4657` and Contact is empty.
- **Root Cause**: `Customer::setPhone()` and `Customer::setCccd()` only call `checkedText()`, which validates that the string is non-empty and contains no pipe characters (`|`), without validating numeric constraints or length.

### 2.2. Desynchronized Initial State between Invoices and Payments
- **Location**: [data/invoices.txt](file:///home/tungle2202/ngu/btl-ktlt2/data/invoices.txt) and [data/payments.txt](file:///home/tungle2202/ngu/btl-ktlt2/data/payments.txt)
- **Severity**: **Medium**
- **Description**:
  - All 10 invoices in `data/invoices.txt` (`HDON001` through `HDON010`) are marked with status `Chua thanh toan` (Unpaid).
  - However, all 10 corresponding payments in `data/payments.txt` (`TT001` through `TT010`) have status `Thanh cong` (Successful).
  - The business logic expects that invoices with successful payments should be `Da thanh toan` (Paid).
- **Root Cause**: The status synchronization function `syncInvoiceStatus()` is only executed when a payment is actively created or updated through the UI menu (UC7). No initial synchronization is performed upon repository initialization.

---

## 3. Business Logic & Runtime Flaws

### 3.1. Silent Data Loss on Renewal Modification (UC6)
- **Location**: [src/use_cases/UC6_Renewals.cpp](file:///home/tungle2202/ngu/btl-ktlt2/src/use_cases/UC6_Renewals.cpp#L128-L134) (`menuRenewals`, case 3)
- **Severity**: **High**
- **Description**: When updating an existing renewal record via menu option 3:
  ```cpp
  Renewal* cur = app.renewals.findById(readLine("Nhap ma gia han can cap nhat: "));
  if (!cur) { cout << "Khong tim thay ban ghi!\n"; continue; }
  cur->updateFields();
  if (Contract* c = app.contracts.findById(cur->getContractId())) c->setEndDate(cur->getNewEndDate());
  cout << "=> Cap nhat gia han thanh cong!\n";
  ```
  Neither `app.renewals.save()` nor `app.contracts.save()` is called. Any edits to renewal details or contract termination dates exist solely in transient memory and are discarded when the program terminates.

### 3.2. Commented-out Foreign Key Validation in Payment Intake (UC7)
- **Location**: [src/use_cases/UC7_Payments.cpp](file:///home/tungle2202/ngu/btl-ktlt2/src/use_cases/UC7_Payments.cpp#L87-L93) (`menuPayments`, case 2, line 90)
- **Severity**: **High**
- **Description**: Line 90 has the invoice existence validation commented out:
  ```cpp
  Payment p; p.inputNew();
  if (app.payments.findById(p.getId())) throw invalid_argument("Ma dinh danh da ton tai.");
  // if (!app.invoices.findById(p.getInvoiceId())) throw invalid_argument("Ma hoa don khong ton tai.");
  app.payments.add(p);
  ```
  Users can input non-existent invoice IDs, resulting in orphaned payments. Conversely, option 3 (updating payment) enforces this check (line 99), creating inconsistent business behavior.

### 3.3. Naive Invoice Status Synchronization Logic
- **Location**: [src/use_cases/UC7_Payments.cpp](file:///home/tungle2202/ngu/btl-ktlt2/src/use_cases/UC7_Payments.cpp#L58-L67) (`syncInvoiceStatus`)
- **Severity**: **High**
- **Description**: 
  ```cpp
  void syncInvoiceStatus(AppData& app, const string& invId) {
      Invoice* inv = app.invoices.findById(invId);
      if (!inv) return;
      bool paid = false;
      for (size_t i = 0; i < app.payments.all().size(); ++i)
          if (app.payments.all()[i].getInvoiceId() == invId &&
              app.payments.all()[i].getStatus() == "Thanh cong") paid = true;
      inv->setStatus(paid ? "Da thanh toan" : "Chua thanh toan");
      app.invoices.save();
  }
  ```
  - The function checks only whether *any* successful payment exists for the invoice.
  - It completely ignores whether the payment amount satisfies the invoice total (e.g. paying 1,000 VND on a 500,000 VND bill marks the invoice fully paid).
  - It does not support cumulative payments (multiple partial payments summing to the total balance).

### 3.4. Unrestricted Entity Deletion Leading to Dangling References
- **Location**: [src/use_cases/UC1_Customers.cpp](file:///home/tungle2202/ngu/btl-ktlt2/src/use_cases/UC1_Customers.cpp#L132-L141) (case 4) and [src/use_cases/UC3_Plans.cpp](file:///home/tungle2202/ngu/btl-ktlt2/src/use_cases/UC3_Plans.cpp#L118-L127) (case 4)
- **Severity**: **High**
- **Description**:
  - In UC1, deleting a customer removes the record from `customers.txt` immediately without verifying whether the customer has active contracts, unpaid invoices, or open incident tickets.
  - In UC3, deleting a tariff plan removes the record from `plans.txt` immediately without verifying whether contracts or registrations are currently assigned to that plan.
  - Result: Foreign-key references across related entities become orphaned/dangling.

### 3.5. Dead Validation Code in Registration Update (UC2)
- **Location**: [src/use_cases/UC2_Registrations.cpp](file:///home/tungle2202/ngu/btl-ktlt2/src/use_cases/UC2_Registrations.cpp#L111-L121)
- **Severity**: **Low**
- **Description**: `Registration::updateFields()` only prompts for `status_`. However, `menuRegistrations` case 3 executes:
  ```cpp
  if (app.customers.findById(tam.getCustomerId()) == nullptr) {
      throw invalid_argument("Ma khach hang moi khong ton tai!");
  }
  ```
  Because `customerId` and `planId` are never modified in `updateFields()`, these validation checks against "new customer" and "new plan" are dead code and may produce confusing error messages if corrupted data exists.

---

## 4. UI & Presentation Defects

### 4.1. Missing Columns in Table Renderers (`displayRow()`)
- **Severity**: **Medium**
- **Description**: Several domain entities fail to display essential attributes in their console tabular output:
  1. `PlanChange::displayRow()` ([src/use_cases/UC4_PlanChanges.cpp#L51-L59](file:///home/tungle2202/ngu/btl-ktlt2/src/use_cases/UC4_PlanChanges.cpp#L51-L59)): Does not render `oldPlanId_`.
  2. `Contract::displayRow()` ([src/use_cases/UC5_Contracts.cpp#L51-L59](file:///home/tungle2202/ngu/btl-ktlt2/src/use_cases/UC5_Contracts.cpp#L51-L59)): Does not render `startDate_`.
  3. `Incident::displayRow()` ([src/use_cases/UC9_Incidents.cpp#L29-L36](file:///home/tungle2202/ngu/btl-ktlt2/src/use_cases/UC9_Incidents.cpp#L29-L36)): Does not render `customerId_`.
  4. `Support::displayRow()` ([src/use_cases/UC10_Supports.cpp#L64-L71](file:///home/tungle2202/ngu/btl-ktlt2/src/use_cases/UC10_Supports.cpp#L64-L71)): Does not render `incidentId_`, `technician_`, or `note_`. Furthermore, `{history_, 0}` specifies a column width of 0, corrupting adjacent spacing.

---

## 5. Memory Safety & Architecture Concerns

### 5.1. Raw Pointer Invalidation in `Repository<T>::findById()`
- **Location**: [include/Repository.h](file:///home/tungle2202/ngu/btl-ktlt2/include/Repository.h#L71-L84)
- **Severity**: **Medium**
- **Description**:
  ```cpp
  T* findById(const string& id) {
      for (auto& record : records_) {
          if (record.getId() == id) return &record;
      }
      return nullptr;
  }
  ```
  `findById()` returns a raw pointer into the dynamically resized `std::vector<T> records_`. If any operation pushes a new record or modifies vector capacity while a pointer is retained, vector reallocation invalidates all active pointers, leading to undefined behavior or segmentation faults upon dereference.

### 5.2. Search Logic Vulnerable to False Positives
- **Location**: `UC1_Customers.cpp` (case 5), `UC3_Plans.cpp` (case 5), `UC10_Supports.cpp` (choice 6)
- **Severity**: **Low**
- **Description**: Search filtering relies on `field.find(keyword) != string::npos`. Searching for common characters (e.g., `1` or `a`) returns matches across IDs, phone numbers, CCCD, and names indiscriminately without token boundary isolation.
