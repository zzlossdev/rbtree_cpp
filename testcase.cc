#include "testcase.h"
#include <span>

map<string_view, TestcaseBase *> TestcaseBase::smTestCases_ = {};

int main(int argc, const char *argv[]) {
  auto args = span(argv + 1, argc - 1);
  if (argc == 1) {
    TestcaseBase::TestAllCases();
  } else {
    for (auto arg : args) {
      string_view option(arg);
      if (option == "-i" || option == "--interactive") {
        auto i = -1;
        auto num = TestcaseBase::PrintAllTestcases();
        while (true) {
          if (cin.peek() == '\n') {
            TestcaseBase::TestAllCases();
            break;
          } else {
            cin >> i;
            if (cin.good() && i >= 0 && i < num) {
              TestcaseBase::GetTestcaseByIndex(i)->testRoutine();
              cin.ignore(numeric_limits<streamsize>::max(), '\n');
              break;
            } else {
              cin.clear();
              cin.ignore(numeric_limits<streamsize>::max(), '\n');
              cout << "invalid input, make sure you choose num from [0 to "
                   << num << ")" << endl;
            }
          }
        }
        break;
      }
    }
  }

  TestcaseBase::FreeAllCases();

  return 0;
}
