/* Generated file, do not edit */

#ifndef CXXTEST_RUNNING
#define CXXTEST_RUNNING
#endif

#include <cxxtest/TestListener.h>
#include <cxxtest/TestTracker.h>
#include <cxxtest/TestRunner.h>
#include <cxxtest/RealDescriptions.h>
#include <cxxtest/ParenPrinter.h>
#include <cxxtest/Win32Gui.h>

int main( int argc, char *argv[] ) {
 return CxxTest::GuiTuiRunner<CxxTest::Win32Gui, CxxTest::ParenPrinter>( argc, argv ).run();
}
#include "test_sqw.h"

static tmain suite_tmain;

static CxxTest::List Tests_tmain = { 0, 0 };
CxxTest::StaticSuiteDescription suiteDescription_tmain( "test_sqw.h", 5, "tmain", suite_tmain, Tests_tmain );

static class TestDescription_tmain_testTMain : public CxxTest::RealTestDescription {
public:
 TestDescription_tmain_testTMain() : CxxTest::RealTestDescription( Tests_tmain, suiteDescription_tmain, 8, "testTMain" ) {}
 void runTest() { suite_tmain.testTMain(); }
} testDescription_tmain_testTMain;

#include <cxxtest/Root.cpp>
