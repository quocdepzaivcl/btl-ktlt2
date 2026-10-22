#include "Menus.h"
#include <iostream>
#include <cassert>
#include <vector>
#include <string>
#include <sstream>

using namespace std;

// ANSI Colors for test reporter
#define COLOR_RESET   "\033[0m"
#define COLOR_GREEN   "\033[32m"
#define COLOR_RED     "\033[31m"
#define COLOR_YELLOW  "\033[33m"
#define COLOR_CYAN    "\033[36m"
#define COLOR_BOLD    "\033[1m"

struct TestStats {
    int total = 0;
    int passed = 0;
    int failed = 0;
    vector<string> failureDetails;
    vector<string> detectedProblems;
};

static TestStats g_stats;

#define TEST_ASSERT(cond, msg) do { \
    g_stats.total++; \
    if (cond) { \
        g_stats.passed++; \
    } else { \
        g_stats.failed++; \
        stringstream ss; \
        ss << __FILE__ << ":" << __LINE__ << " [FAILED] " << msg; \
        g_stats.failureDetails.push_back(ss.str()); \
        cout << COLOR_RED << "  FAIL: " << msg << COLOR_RESET << "\n"; \
    } \
} while(0)

#define RECORD_PROBLEM(category, description) do { \
    stringstream ss; \
    ss << "[" << category << "] " << description; \
    g_stats.detectedProblems.push_back(ss.str()); \
} while(0)

// ==========================================
// 1. Validator Tests
// ==========================================
void test_validators() {
    cout << COLOR_CYAN << "Running Validator Unit Tests..." << COLOR_RESET << "\n";

    // trimText
    TEST_ASSERT(trimText("   hello   ") == "hello", "trimText whitespace trimming");
    TEST_ASSERT(trimText("\t\r\nworld\n\t") == "world", "trimText tab and newline trimming");
    TEST_ASSERT(trimText("   ") == "", "trimText empty when all whitespace");
    TEST_ASSERT(trimText("") == "", "trimText empty string");

    // validateText
    bool threw = false;
    try { validateText("", "Field", false); } catch (const invalid_argument&) { threw = true; }
    TEST_ASSERT(threw, "validateText throws on empty string when required");

    threw = false;
    try { validateText("", "Field", true); } catch (const invalid_argument&) { threw = true; }
    TEST_ASSERT(!threw, "validateText allows empty string when allowEmpty=true");

    threw = false;
    try { validateText("hello|world", "Field"); } catch (const invalid_argument&) { threw = true; }
    TEST_ASSERT(threw, "validateText throws on pipe delimiter in input");

    threw = false;
    try { validateText("hello\nworld", "Field"); } catch (const invalid_argument&) { threw = true; }
    TEST_ASSERT(threw, "validateText throws on newline in input");

    // validatePositiveNumber
    threw = false;
    try { validatePositiveNumber("150000", "Price"); } catch (const exception&) { threw = true; }
    TEST_ASSERT(!threw, "validatePositiveNumber accepts valid positive integer");

    threw = false;
    try { validatePositiveNumber("0", "Price"); } catch (const invalid_argument&) { threw = true; }
    TEST_ASSERT(threw, "validatePositiveNumber rejects zero");

    threw = false;
    try { validatePositiveNumber("-100", "Price"); } catch (const invalid_argument&) { threw = true; }
    TEST_ASSERT(threw, "validatePositiveNumber rejects negative numbers");

    threw = false;
    try { validatePositiveNumber("abc", "Price"); } catch (const invalid_argument&) { threw = true; }
    TEST_ASSERT(threw, "validatePositiveNumber rejects alphabetic text");

    // isValidDateText
    TEST_ASSERT(isValidDateText("2026-05-15"), "isValidDateText accepts YYYY-MM-DD");
    TEST_ASSERT(isValidDateText("15/05/2026"), "isValidDateText accepts DD/MM/YYYY");
    TEST_ASSERT(isValidDateText("2024-02-29"), "isValidDateText accepts leap day in leap year 2024");
    TEST_ASSERT(!isValidDateText("2023-02-29"), "isValidDateText rejects leap day in non-leap year 2023");
    TEST_ASSERT(!isValidDateText("2026-04-31"), "isValidDateText rejects day 31 in 30-day month April");
    TEST_ASSERT(!isValidDateText("2026-13-01"), "isValidDateText rejects month 13");
    TEST_ASSERT(!isValidDateText("invalid-date"), "isValidDateText rejects gibberish text");
}

// ==========================================
// 2. Tokenizer & Parser Tests
// ==========================================
void test_tokenizers() {
    cout << COLOR_CYAN << "Running Tokenizer Unit Tests..." << COLOR_RESET << "\n";

    string fields[4];
    string line = "F1|F2|F3|F4";
    TEST_ASSERT(splitPipe(line, fields, 4), "splitPipe parses 4 exact fields");
    TEST_ASSERT(fields[0] == "F1" && fields[1] == "F2" && fields[2] == "F3" && fields[3] == "F4", "splitPipe field values match");

    TEST_ASSERT(!splitPipe(line, fields, 3), "splitPipe rejects line when fewer fields requested than present");
    TEST_ASSERT(!splitPipe(line, fields, 5), "splitPipe rejects line when more fields requested than present");

    string crlfLine = "A|B|C|D\r";
    TEST_ASSERT(splitPipe(crlfLine, fields, 4), "splitPipe handles trailing CRLF carriage return");
    TEST_ASSERT(fields[3] == "D", "splitPipe trims trailing \\r from last field");
}

// ==========================================
// 3. Entity Serialization Round-Trip Tests
// ==========================================
void test_entities_serialization() {
    cout << COLOR_CYAN << "Running Entity Serialization Round-Trip Tests..." << COLOR_RESET << "\n";

    // Customer
    Customer c;
    string cLine = "KH999|Test User|0987654321|001201999999|Hanoi|test@example.com";
    TEST_ASSERT(c.fromFileString(cLine), "Customer::fromFileString succeeds on valid record");
    TEST_ASSERT(c.toFileString() == cLine, "Customer::toFileString round-trip equality");

    // Plan
    Plan p;
    string pLine = "GOI999|Test Plan|500 Mbps|350000|30 ngay";
    TEST_ASSERT(p.fromFileString(pLine), "Plan::fromFileString succeeds on valid record");
    TEST_ASSERT(p.toFileString() == pLine, "Plan::toFileString round-trip equality");

    // Registration
    Registration r;
    string rLine = "DK999|KH999|GOI999|2026-05-01|Da lap dat";
    TEST_ASSERT(r.fromFileString(rLine), "Registration::fromFileString succeeds on valid record");
    TEST_ASSERT(r.toFileString() == rLine, "Registration::toFileString round-trip equality");

    // Contract
    Contract ct;
    string ctLine = "HD999|KH999|GOI999|2026-05-01|2027-05-01|Dang hoat dong";
    TEST_ASSERT(ct.fromFileString(ctLine), "Contract::fromFileString succeeds on valid record");
    TEST_ASSERT(ct.toFileString() == ctLine, "Contract::toFileString round-trip equality");

    // PlanChange
    PlanChange pc;
    string pcLine = "DC999|HD999|GOI001|GOI002|2026-06-01|Da thuc hien";
    TEST_ASSERT(pc.fromFileString(pcLine), "PlanChange::fromFileString succeeds on valid record");
    TEST_ASSERT(pc.toFileString() == pcLine, "PlanChange::toFileString round-trip equality");

    // Renewal
    Renewal rn;
    string rnLine = "GH999|HD999|2027-05-01|2027-08-01|3|Thanh cong";
    TEST_ASSERT(rn.fromFileString(rnLine), "Renewal::fromFileString succeeds on valid record");
    TEST_ASSERT(rn.toFileString() == rnLine, "Renewal::toFileString round-trip equality");

    // Payment
    Payment pay;
    string payLine = "TT999|HDON999|350000|Thanh cong";
    TEST_ASSERT(pay.fromFileString(payLine), "Payment::fromFileString succeeds on valid record");
    TEST_ASSERT(pay.toFileString() == payLine, "Payment::toFileString round-trip equality");

    // Invoice
    Invoice inv;
    string invLine = "HDON999|KH999|350000|Chua thanh toan";
    TEST_ASSERT(inv.fromFileString(invLine), "Invoice::fromFileString succeeds on valid record");
    TEST_ASSERT(inv.toFileString() == invLine, "Invoice::toFileString round-trip equality");

    // Incident
    Incident inc;
    string incLine = "SC999|HD999|KH999|Mat ket noi|Moi tiep nhan";
    TEST_ASSERT(inc.fromFileString(incLine), "Incident::fromFileString succeeds on valid record");
    TEST_ASSERT(inc.toFileString() == incLine, "Incident::toFileString round-trip equality");

    // Support
    Support sp;
    string spLine = "HT999|KH999|SC999|KTV A|Ghi chu|Lich su|Cho phan cong";
    TEST_ASSERT(sp.fromFileString(spLine), "Support::fromFileString succeeds on valid record");
    TEST_ASSERT(sp.toFileString() == spLine, "Support::toFileString round-trip equality");
}

// ==========================================
// 4. In-Memory Repository Tests
// ==========================================
void test_repository_logic() {
    cout << COLOR_CYAN << "Running Repository In-Memory Logic Tests..." << COLOR_RESET << "\n";

    Repository<Customer> repo(""); // in-memory only (path is empty)
    TEST_ASSERT(repo.empty(), "New in-memory repository is empty");
    TEST_ASSERT(repo.size() == 0, "New in-memory repository size is 0");

    Customer c1;
    c1.setId("TEST01");
    c1.setFullName("User 1");
    c1.setPhone("0901234567");
    c1.setCccd("001201111111");
    c1.setAddress("Hanoi");

    TEST_ASSERT(repo.add(c1), "Adding new record succeeds");
    TEST_ASSERT(repo.size() == 1, "Repository size incremented to 1");
    TEST_ASSERT(repo.findById("TEST01") != nullptr, "findById locates newly added record");

    Customer cDup;
    cDup.setId("TEST01");
    cDup.setFullName("Duplicate User");
    cDup.setPhone("0909999999");
    cDup.setCccd("001201999999");
    cDup.setAddress("Hanoi");
    TEST_ASSERT(!repo.add(cDup), "Repository rejects adding duplicate ID");
    TEST_ASSERT(repo.size() == 1, "Repository size remains 1 after rejected duplicate");

    TEST_ASSERT(repo.removeById("TEST01"), "removeById removes existing record");
    TEST_ASSERT(repo.size() == 0, "Repository size decremented to 0 after removal");
    TEST_ASSERT(!repo.removeById("TEST01"), "removeById returns false for non-existent ID");
}

// ==========================================
// 5. Database & Referential Integrity Audit
// ==========================================
void test_database_and_referential_integrity() {
    cout << COLOR_CYAN << "Auditing Persistent Database & Referential Integrity..." << COLOR_RESET << "\n";

    AppData app;

    // Check size of repositories
    TEST_ASSERT(app.customers.size() >= 10, "Customers repository loaded at least 10 records");
    TEST_ASSERT(app.plans.size() >= 10, "Plans repository loaded at least 10 records");
    TEST_ASSERT(app.registrations.size() >= 10, "Registrations repository loaded at least 10 records");
    TEST_ASSERT(app.contracts.size() >= 10, "Contracts repository loaded at least 10 records");
    TEST_ASSERT(app.planChanges.size() >= 10, "PlanChanges repository loaded at least 10 records");
    TEST_ASSERT(app.renewals.size() >= 10, "Renewals repository loaded at least 10 records");
    TEST_ASSERT(app.payments.size() >= 10, "Payments repository loaded at least 10 records");
    TEST_ASSERT(app.invoices.size() >= 10, "Invoices repository loaded at least 10 records");
    TEST_ASSERT(app.incidents.size() >= 10, "Incidents repository loaded at least 10 records");
    TEST_ASSERT(app.supports.size() >= 10, "Supports repository loaded at least 10 records");

    // Referential Integrity Check: Registrations -> Customers, Plans
    for (const auto& reg : app.registrations.all()) {
        bool custExists = (app.customers.findById(reg.getCustomerId()) != nullptr);
        TEST_ASSERT(custExists, "Registration customerId " + reg.getCustomerId() + " exists in customers");
        bool planExists = (app.plans.findById(reg.getPlanId()) != nullptr);
        TEST_ASSERT(planExists, "Registration planId " + reg.getPlanId() + " exists in plans");
    }

    // Referential Integrity Check: Contracts -> Customers, Plans
    for (const auto& c : app.contracts.all()) {
        bool custExists = (app.customers.findById(c.getCustomerId()) != nullptr);
        TEST_ASSERT(custExists, "Contract customerId " + c.getCustomerId() + " exists in customers");
        bool planExists = (app.plans.findById(c.getPlanId()) != nullptr);
        TEST_ASSERT(planExists, "Contract planId " + c.getPlanId() + " exists in plans");
    }

    // Referential Integrity Check: PlanChanges -> Contracts, Plans
    for (const auto& pc : app.planChanges.all()) {
        bool ctExists = (app.contracts.findById(pc.getContractId()) != nullptr);
        TEST_ASSERT(ctExists, "PlanChange contractId " + pc.getContractId() + " exists in contracts");
        bool newPlanExists = (app.plans.findById(pc.getNewPlanId()) != nullptr);
        TEST_ASSERT(newPlanExists, "PlanChange newPlanId " + pc.getNewPlanId() + " exists in plans");
    }

    // Referential Integrity Check: Renewals -> Contracts
    for (const auto& rn : app.renewals.all()) {
        bool ctExists = (app.contracts.findById(rn.getContractId()) != nullptr);
        TEST_ASSERT(ctExists, "Renewal contractId " + rn.getContractId() + " exists in contracts");
    }

    // Referential Integrity Check: Invoices -> Customers
    for (const auto& inv : app.invoices.all()) {
        bool custExists = (app.customers.findById(inv.getCustomerId()) != nullptr);
        TEST_ASSERT(custExists, "Invoice customerId " + inv.getCustomerId() + " exists in customers");
    }

    // Referential Integrity Check: Payments -> Invoices
    for (const auto& p : app.payments.all()) {
        bool invExists = (app.invoices.findById(p.getInvoiceId()) != nullptr);
        TEST_ASSERT(invExists, "Payment invoiceId " + p.getInvoiceId() + " exists in invoices");
    }

    // Referential Integrity Check: Incidents -> Contracts, Customers
    for (const auto& inc : app.incidents.all()) {
        bool ctExists = (app.contracts.findById(inc.getContractId()) != nullptr);
        TEST_ASSERT(ctExists, "Incident contractId " + inc.getContractId() + " exists in contracts");
        bool custExists = (app.customers.findById(inc.getCustomerId()) != nullptr);
        TEST_ASSERT(custExists, "Incident customerId " + inc.getCustomerId() + " exists in customers");
    }

    // Referential Integrity Check: Supports -> Customers, Incidents
    for (const auto& sp : app.supports.all()) {
        bool custExists = (app.customers.findById(sp.getCustomerId()) != nullptr);
        TEST_ASSERT(custExists, "Support customerId " + sp.getCustomerId() + " exists in customers");
        if (!sp.getIncidentId().empty()) {
            bool incExists = (app.incidents.findById(sp.getIncidentId()) != nullptr);
            TEST_ASSERT(incExists, "Support incidentId " + sp.getIncidentId() + " exists in incidents");
        }
    }
}

// ==========================================
// 6. Deep Audit of Data Anomalies & Logic Bugs
// ==========================================
void audit_data_and_logic_problems() {
    cout << COLOR_CYAN << "Auditing Data Anomalies & Business Logic Flaws..." << COLOR_RESET << "\n";

    AppData app;

    // Anomaly 1: Corrupt / Garbage record in data/customers.txt
    Customer* garbageCust = app.customers.findById("213");
    if (garbageCust != nullptr) {
        RECORD_PROBLEM("DATA_CORRUPTION", "data/customers.txt contains a corrupt dummy record '213|4235|5467|4657|4657|' which has invalid phone and CCCD formats but was parsed without error.");
    }

    // Anomaly 2: Invoices and Payments Status Synchronization Inconsistency
    int unlinkedPaidInvoices = 0;
    for (const auto& pay : app.payments.all()) {
        if (pay.getStatus() == "Thanh cong") {
            Invoice* inv = app.invoices.findById(pay.getInvoiceId());
            if (inv && inv->getStatus() == "Chua thanh toan") {
                unlinkedPaidInvoices++;
            }
        }
    }
    if (unlinkedPaidInvoices > 0) {
        string desc = "Found " + to_string(unlinkedPaidInvoices) + " invoices marked as 'Chua thanh toan' in data/invoices.txt despite having successful payments ('Thanh cong') in data/payments.txt. Database is in a desynchronized initial state.";
        RECORD_PROBLEM("DATA_INCONSISTENCY", desc);
    }

    // Logic Bug 3: UC6 Renewal Update doesn't save to file
    RECORD_PROBLEM("LOGIC_BUG", "In UC6_Renewals.cpp (menuRenewals, case 3), when updating an existing renewal record, neither app.renewals.save() nor app.contracts.save() is called, so renewal modifications are silently lost on exit.");

    // Logic Bug 4: UC7 Payment Creation skips Invoice verification
    RECORD_PROBLEM("LOGIC_BUG", "In UC7_Payments.cpp (menuPayments, case 2, line 90), the check 'if (!app.invoices.findById(p.getInvoiceId()))' is commented out, allowing users to register payments for non-existent invoices.");

    // Logic Bug 5: syncInvoiceStatus ignores partial / mismatched payment amounts
    RECORD_PROBLEM("LOGIC_BUG", "In UC7_Payments.cpp (syncInvoiceStatus), any payment with status 'Thanh cong' immediately sets the invoice to 'Da thanh toan', regardless of whether the payment amount matches or is less than the invoice amount.");

    // Logic Bug 6: Deleting Customers / Plans bypasses foreign-key cascade or restriction checks
    RECORD_PROBLEM("LOGIC_BUG", "In UC1_Customers.cpp (case 4) and UC3_Plans.cpp (case 4), customers and plans can be deleted without verifying whether they are currently referenced by active contracts, invoices, or incidents, causing dangling foreign keys.");

    // Logic Bug 7: Display columns missing critical fields
    RECORD_PROBLEM("UI_DEFECT", "PlanChange::displayRow() does not display oldPlanId_; Contract::displayRow() does not display startDate_; Incident::displayRow() does not display customerId_; Support::displayRow() does not display incidentId_, technician_, or note_.");

    // Logic Bug 8: Vector pointer invalidation in Repository<T>::findById()
    RECORD_PROBLEM("CONCURRENCY_SAFETY", "Repository<T>::findById() returns raw pointers T* directly into std::vector<T> records_. Calling add() causes reallocation which invalidates any active pointers.");
}

// ==========================================
// Main Test Runner Entry Point
// ==========================================
int main() {
    cout << COLOR_BOLD << "========================================================\n"
         << "       FTTH PROJECT AUTOMATED TEST SUITE (C++17)        \n"
         << "========================================================" << COLOR_RESET << "\n\n";

    test_validators();
    test_tokenizers();
    test_entities_serialization();
    test_repository_logic();
    test_database_and_referential_integrity();
    audit_data_and_logic_problems();

    cout << "\n" << COLOR_BOLD << "========================================================\n"
         << "                     TEST SUMMARY                       \n"
         << "========================================================" << COLOR_RESET << "\n";
    cout << "Total Assertions: " << g_stats.total << "\n";
    cout << COLOR_GREEN << "Passed:           " << g_stats.passed << COLOR_RESET << "\n";

    if (g_stats.failed > 0) {
        cout << COLOR_RED << "Failed:           " << g_stats.failed << COLOR_RESET << "\n";
        cout << "\n" << COLOR_RED << "Failure details:" << COLOR_RESET << "\n";
        for (const auto& f : g_stats.failureDetails) {
            cout << "  - " << f << "\n";
        }
    } else {
        cout << COLOR_GREEN << "All Unit & Integration Assertions Passed Successfully!" << COLOR_RESET << "\n";
    }

    if (!g_stats.detectedProblems.empty()) {
        cout << "\n" << COLOR_YELLOW << "Detected Codebase / Database Problems (" << g_stats.detectedProblems.size() << "):" << COLOR_RESET << "\n";
        for (const auto& p : g_stats.detectedProblems) {
            cout << "  * " << p << "\n";
        }
    }

    cout << "========================================================\n";

    return (g_stats.failed == 0) ? 0 : 1;
}
