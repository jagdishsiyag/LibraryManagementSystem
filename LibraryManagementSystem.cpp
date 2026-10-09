/*  ============================================================
    LIBRARY MANAGEMENT SYSTEM
    Console-based application using OOP + File Handling (C++)
    ------------------------------------------------------------
    OOP Concepts:
        * Encapsulation  -> Book / Member protect their data
        * Abstraction    -> Library exposes clean operations
        * Inheritance    -> Person (base) -> Member / Librarian
        * Polymorphism   -> virtual display(), getRole()
        * Composition    -> Library "has-a" vector<Book>, vector<Member>,
                             vector<IssueRecord>

    Features : Add / Display / Search (title or author) / Issue /
               Return / View issued records
    Storage  : books.txt, members.txt, issues.txt
    ============================================================ */

#include <iostream>
#include <fstream>
#include <sstream>
#include <vector>
#include <string>
#include <memory>
#include <iomanip>
#include <limits>
#include <algorithm>
#include <ctime>

using namespace std;

const string BOOKS_FILE   = "books.txt";
const string MEMBERS_FILE = "members.txt";
const string ISSUES_FILE  = "issues.txt";

/* ---------------- Validated Input Helpers ---------------- */

int readInt(const string& prompt) {
    int value;
    while (true) {
        cout << prompt;
        if (cin >> value) {
            cin.ignore(numeric_limits<streamsize>::max(), '\n');
            return value;
        }
        cin.clear();
        cin.ignore(numeric_limits<streamsize>::max(), '\n');
        cout << "  [!] Invalid input. Enter a whole number.\n";
    }
}

string readLine(const string& prompt) {
    string s;
    cout << prompt;
    getline(cin, s);
    return s;
}

// Turns a string into a lowercase copy (for case-insensitive search).
string toLower(const string& s) {
    string out = s;
    transform(out.begin(), out.end(), out.begin(),
              [](unsigned char c){ return static_cast<char>(tolower(c)); });
    return out;
}

// Returns true if 'needle' appears inside 'haystack' (case-insensitive).
bool contains(const string& haystack, const string& needle) {
    return toLower(haystack).find(toLower(needle)) != string::npos;
}

// Current date as YYYY-MM-DD
string today() {
    time_t t = time(nullptr);
    tm* lt = localtime(&t);
    char buf[11];
    strftime(buf, sizeof(buf), "%Y-%m-%d", lt);
    return string(buf);
}

/* ================= Class : Book ================= */

class Book {
private:
    int    id;
    string title;
    string author;
    bool   issued;                    // true if currently borrowed

public:
    Book() : id(0), issued(false) {}
    Book(int id_, const string& t, const string& a, bool is = false)
        : id(id_), title(t), author(a), issued(is) {}

    int    getId()     const { return id; }
    string getTitle()  const { return title; }
    string getAuthor() const { return author; }
    bool   isIssued()  const { return issued; }

    void setTitle(const string& t)  { title = t; }
    void setAuthor(const string& a) { author = a; }
    void setIssued(bool s)          { issued = s; }

    void display() const {
        cout << left
             << setw(8)  << id
             << setw(34) << title
             << setw(24) << author
             << setw(10) << (issued ? "Issued" : "Available")
             << '\n';
    }

    string serialize() const {
        ostringstream oss;
        oss << id << '|' << title << '|' << author << '|' << (issued ? 1 : 0);
        return oss.str();
    }

    static Book deserialize(const string& line) {
        stringstream ss(line);
        string idS, title, author, issuedS;
        getline(ss, idS,     '|');
        getline(ss, title,   '|');
        getline(ss, author,  '|');
        getline(ss, issuedS, '|');
        return Book(stoi(idS), title, author, issuedS == "1");
    }
};

/* ================= Base Class : Person ================= */

class Person {
protected:
    int    id;
    string name;

public:
    Person(int id_, const string& n) : id(id_), name(n) {}
    virtual ~Person() {}

    int    getId()   const { return id; }
    string getName() const { return name; }
    void   setName(const string& n) { name = n; }

    virtual string getRole() const = 0;

    virtual void display() const {
        cout << left << setw(8) << id << setw(28) << name << getRole() << '\n';
    }

    virtual string serialize() const {
        ostringstream oss;
        oss << id << '|' << name;
        return oss.str();
    }
};

/* ================= Derived Class : Member ================= */

class Member : public Person {
private:
    string phone;
    int    booksIssued;               // number of books currently held

public:
    static constexpr int MAX_BOOKS = 3;

    Member(int id_, const string& n, const string& p, int issued = 0)
        : Person(id_, n), phone(p), booksIssued(issued) {}

    string getRole() const override { return "Member"; }
    string getPhone() const { return phone; }
    int getBooksIssued() const { return booksIssued; }

    void setPhone(const string& p) { phone = p; }
    void incrementIssued() { ++booksIssued; }
    void decrementIssued() { if (booksIssued > 0) --booksIssued; }

    bool canBorrowMore() const { return booksIssued < MAX_BOOKS; }

    void display() const override {
        cout << left
             << setw(8)  << id
             << setw(28) << name
             << setw(14) << phone
             << setw(12) << booksIssued
             << '\n';
    }

    string serialize() const override {
        ostringstream oss;
        oss << id << '|' << name << '|' << phone << '|' << booksIssued;
        return oss.str();
    }

    static Member deserialize(const string& line) {
        stringstream ss(line);
        string idS, name, phone, issuedS;
        getline(ss, idS,     '|');
        getline(ss, name,    '|');
        getline(ss, phone,   '|');
        getline(ss, issuedS, '|');
        int issued = issuedS.empty() ? 0 : stoi(issuedS);
        return Member(stoi(idS), name, phone, issued);
    }
};

/* ================= Struct : IssueRecord ================= */

struct IssueRecord {
    int    bookId;
    int    memberId;
    string issueDate;
    string returnDate;                // empty if not yet returned

    bool isReturned() const { return !returnDate.empty(); }

    string serialize() const {
        ostringstream oss;
        oss << bookId << '|' << memberId << '|'
            << issueDate << '|' << returnDate;
        return oss.str();
    }

    static IssueRecord deserialize(const string& line) {
        stringstream ss(line);
        string b, m, iD, rD;
        getline(ss, b,  '|');
        getline(ss, m,  '|');
        getline(ss, iD, '|');
        getline(ss, rD, '|');
        return IssueRecord{ stoi(b), stoi(m), iD, rD };
    }
};

/* ================= Class : Library ================= */

class Library {
private:
    vector<Book>        books;
    vector<Member>      members;
    vector<IssueRecord> issues;

    int nextBookId;
    int nextMemberId;

    /* ---- lookups ---- */

    int findBookIndex(int id) const {
        for (size_t i = 0; i < books.size(); ++i)
            if (books[i].getId() == id) return static_cast<int>(i);
        return -1;
    }

    int findMemberIndex(int id) const {
        for (size_t i = 0; i < members.size(); ++i)
            if (members[i].getId() == id) return static_cast<int>(i);
        return -1;
    }

    // Finds the active (unreturned) issue record for a book, or -1.
    int findActiveIssue(int bookId) const {
        for (size_t i = 0; i < issues.size(); ++i)
            if (issues[i].bookId == bookId && !issues[i].isReturned())
                return static_cast<int>(i);
        return -1;
    }

    void recomputeIds() {
        int maxB = 1000, maxM = 5000;
        for (const auto& b : books)   maxB = max(maxB, b.getId());
        for (const auto& m : members) maxM = max(maxM, m.getId());
        nextBookId   = maxB + 1;
        nextMemberId = maxM + 1;
    }

    /* ---- file I/O ---- */

    void loadBooks() {
        books.clear();
        ifstream fin(BOOKS_FILE);
        string line;
        while (getline(fin, line)) {
            if (line.empty()) continue;
            try { books.push_back(Book::deserialize(line)); } catch (...) {}
        }
    }

    void loadMembers() {
        members.clear();
        ifstream fin(MEMBERS_FILE);
        string line;
        while (getline(fin, line)) {
            if (line.empty()) continue;
            try { members.push_back(Member::deserialize(line)); } catch (...) {}
        }
    }

    void loadIssues() {
        issues.clear();
        ifstream fin(ISSUES_FILE);
        string line;
        while (getline(fin, line)) {
            if (line.empty()) continue;
            try { issues.push_back(IssueRecord::deserialize(line)); } catch (...) {}
        }
    }

    void saveBooks() const {
        ofstream fout(BOOKS_FILE, ios::trunc);
        for (const auto& b : books) fout << b.serialize() << '\n';
    }

    void saveMembers() const {
        ofstream fout(MEMBERS_FILE, ios::trunc);
        for (const auto& m : members) fout << m.serialize() << '\n';
    }

    void saveIssues() const {
        ofstream fout(ISSUES_FILE, ios::trunc);
        for (const auto& r : issues) fout << r.serialize() << '\n';
    }

    void saveAll() const {
        saveBooks();
        saveMembers();
        saveIssues();
    }

public:
    Library() {
        loadBooks();
        loadMembers();
        loadIssues();
        recomputeIds();
    }

    ~Library() { saveAll(); }

    size_t bookCount()   const { return books.size(); }
    size_t memberCount() const { return members.size(); }

    /* ================= Book Operations ================= */

    void addBook() {
        cout << "\n--- Add Book ---\n";
        string title = readLine("Enter Book Title  : ");
        if (title.empty()) { cout << "  [!] Title cannot be empty.\n"; return; }
        string author = readLine("Enter Author Name : ");
        if (author.empty()) { cout << "  [!] Author cannot be empty.\n"; return; }

        int id = nextBookId++;
        books.emplace_back(id, title, author, false);
        saveBooks();

        cout << "  [OK] Book added successfully. Book ID = " << id << '\n';
    }

    void displayBooks() const {
        cout << "\n--- All Books ---\n";
        if (books.empty()) { cout << "  No books in the library.\n"; return; }

        cout << left
             << setw(8)  << "Book ID"
             << setw(34) << "Title"
             << setw(24) << "Author"
             << setw(10) << "Status" << '\n';
        cout << string(76, '-') << '\n';

        for (const auto& b : books) b.display();

        cout << string(76, '-') << '\n';
        cout << "Total books: " << books.size() << '\n';
    }

    void searchBook() const {
        cout << "\n--- Search Book ---\n";
        cout << "  1. Search by Title\n";
        cout << "  2. Search by Author\n";
        int choice = readInt("Enter choice (1-2) : ");
        if (choice != 1 && choice != 2) {
            cout << "  [!] Invalid choice.\n";
            return;
        }

        string query = readLine(
            choice == 1 ? "Enter title keyword  : " : "Enter author keyword : ");
        if (query.empty()) {
            cout << "  [!] Search keyword cannot be empty.\n";
            return;
        }

        cout << "\n--- Search Results ---\n";
        bool found = false;
        cout << left
             << setw(8)  << "Book ID"
             << setw(34) << "Title"
             << setw(24) << "Author"
             << setw(10) << "Status" << '\n';
        cout << string(76, '-') << '\n';

        for (const auto& b : books) {
            const string& field = (choice == 1) ? b.getTitle() : b.getAuthor();
            if (contains(field, query)) {
                b.display();
                found = true;
            }
        }
        if (!found) cout << "  No matching books found.\n";
        cout << string(76, '-') << '\n';
    }

    /* ================= Member Operations ================= */

    void addMember() {
        cout << "\n--- Add Member ---\n";
        string name = readLine("Enter Member Name  : ");
        if (name.empty()) { cout << "  [!] Name cannot be empty.\n"; return; }
        string phone = readLine("Enter Phone Number : ");

        int id = nextMemberId++;
        members.emplace_back(id, name, phone, 0);
        saveMembers();

        cout << "  [OK] Member added successfully. Member ID = " << id << '\n';
    }

    void displayMembers() const {
        cout << "\n--- All Members ---\n";
        if (members.empty()) { cout << "  No members registered.\n"; return; }

        cout << left
             << setw(8)  << "Member ID"
             << setw(28) << "Name"
             << setw(14) << "Phone"
             << setw(12) << "Books Held" << '\n';
        cout << string(62, '-') << '\n';

        for (const auto& m : members) m.display();

        cout << string(62, '-') << '\n';
        cout << "Total members: " << members.size() << '\n';
    }

    /* ================= Issue / Return ================= */

    void issueBook() {
        cout << "\n--- Issue Book ---\n";

        int bookId = readInt("Enter Book ID   : ");
        int bIdx = findBookIndex(bookId);
        if (bIdx == -1) { cout << "  [!] Book not found.\n"; return; }
        if (books[bIdx].isIssued()) {
            cout << "  [!] This book is already issued.\n";
            return;
        }

        int memId = readInt("Enter Member ID : ");
        int mIdx = findMemberIndex(memId);
        if (mIdx == -1) { cout << "  [!] Member not found.\n"; return; }

        if (!members[mIdx].canBorrowMore()) {
            cout << "  [!] Member already holds " << Member::MAX_BOOKS
                 << " books. Cannot issue more.\n";
            return;
        }

        books[bIdx].setIssued(true);
        members[mIdx].incrementIssued();
        issues.push_back(IssueRecord{bookId, memId, today(), ""});
        saveAll();

        cout << "  [OK] Book \"" << books[bIdx].getTitle()
             << "\" issued to " << members[mIdx].getName()
             << " on " << today() << '\n';
    }

    void returnBook() {
        cout << "\n--- Return Book ---\n";
        int bookId = readInt("Enter Book ID : ");
        int bIdx = findBookIndex(bookId);
        if (bIdx == -1) { cout << "  [!] Book not found.\n"; return; }
        if (!books[bIdx].isIssued()) {
            cout << "  [!] This book is not currently issued.\n";
            return;
        }

        int recIdx = findActiveIssue(bookId);
        if (recIdx == -1) {
            cout << "  [!] No active issue record found. Data may be inconsistent.\n";
            return;
        }

        int memId = issues[recIdx].memberId;
        int mIdx  = findMemberIndex(memId);

        books[bIdx].setIssued(false);
        if (mIdx != -1) members[mIdx].decrementIssued();

        issues[recIdx].returnDate = today();
        saveAll();

        cout << "  [OK] Book \"" << books[bIdx].getTitle() << "\" returned on "
             << today() << ".\n";
    }

    void displayIssuedRecords() const {
        cout << "\n--- Issued Records ---\n";
        if (issues.empty()) { cout << "  No issue history.\n"; return; }

        cout << left
             << setw(8)  << "Book ID"
             << setw(10) << "Member ID"
             << setw(14) << "Issue Date"
             << setw(14) << "Return Date"
             << setw(10) << "Status" << '\n';
        cout << string(56, '-') << '\n';

        for (const auto& r : issues) {
            cout << left
                 << setw(8)  << r.bookId
                 << setw(10) << r.memberId
                 << setw(14) << r.issueDate
                 << setw(14) << (r.isReturned() ? r.returnDate : "-")
                 << setw(10) << (r.isReturned() ? "Returned" : "Active")
                 << '\n';
        }
        cout << string(56, '-') << '\n';
    }
};

/* ================= Menu & Main ================= */

void showMenu() {
    cout << "\n=========================================\n";
    cout << "        LIBRARY MANAGEMENT SYSTEM\n";
    cout << "=========================================\n";
    cout << "  1.  Add Book\n";
    cout << "  2.  Display All Books\n";
    cout << "  3.  Search Book (by Title / Author)\n";
    cout << "  4.  Add Member\n";
    cout << "  5.  Display All Members\n";
    cout << "  6.  Issue Book\n";
    cout << "  7.  Return Book\n";
    cout << "  8.  View Issued Records\n";
    cout << "  9.  Exit\n";
    cout << "=========================================\n";
}

int main() {
    Library lib;

    cout << "Welcome to the Library Management System!\n";
    cout << "Loaded " << lib.bookCount()   << " book(s) and "
                     << lib.memberCount() << " member(s).\n";

    while (true) {
        showMenu();
        int choice = readInt("Enter your choice (1-9) : ");

        switch (choice) {
            case 1: lib.addBook();              break;
            case 2: lib.displayBooks();         break;
            case 3: lib.searchBook();           break;
            case 4: lib.addMember();            break;
            case 5: lib.displayMembers();       break;
            case 6: lib.issueBook();            break;
            case 7: lib.returnBook();           break;
            case 8: lib.displayIssuedRecords(); break;
            case 9:
                cout << "\nAll data saved. Goodbye!\n";
                return 0;
            default:
                cout << "  [!] Invalid choice. Please enter 1-9.\n";
        }
    }
}
