#ifndef __TESTCASE_H__
#define __TESTCASE_H__

#include "types.h"
#include <iostream>
#include <map>
#include <string_view>

using namespace std;

class TestcaseBase {
public:
  void registerCaselet(const char *caselet) { caselets_[caselet] = false; }

  void enableCaselet(const char *caselet) {
    if (caselets_.contains(caselet))
      caselets_[caselet] = true;
  }

  bool caseletEnabled(const char *caselet) {
    return caselets_.contains(caselet) ? caselets_[caselet] : false;
  }

  virtual void testRoutine() {}

  virtual ~TestcaseBase() {}

  void printAllCaselets() {}

  static auto GetTestcaseByIndex(u8 index) {
    auto it = next(smTestCases_.begin(), index);
    return it == smTestCases_.end() ? nullptr : it->second;
  }

  static int PrintAllTestcases() {
    auto i = 0;
    cout << "select the case number you want to test" << endl;
    cout << "\ttest all cases[Enter]." << endl;
    for (auto t : smTestCases_) {
      cout << "\ttest " << t.first << "[" << i++ << "]." << endl;
    }

    return i;
  }

  static void PushCase(const char *name, TestcaseBase *tc) {
    smTestCases_[name] = tc;
  }

  static void TestAllCases() {
    for (auto t : smTestCases_) {
      t.second->testRoutine();
    }
  }

  static void FreeAllCases() {
    for (auto t : smTestCases_) {
      delete t.second;
    }
  }

private:
  static map<string_view, TestcaseBase *> smTestCases_;
  map<string_view, bool> caselets_;
};

#define TEST_IF(testlet, expr)                                                 \
  {                                                                            \
    if (caseletEnabled(#testlet)) {                                            \
      expr;                                                                    \
    }                                                                          \
  }

#define INIT_CASE(class)                                                       \
  static void __attribute__((constructor)) initTestCase() {                    \
    TestcaseBase::PushCase(#class, new class());                               \
  }
#endif
