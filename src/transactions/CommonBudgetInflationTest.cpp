#include "ledger/LedgerDelta.h"
#include "ledger/LedgerManager.h"
#include "lib/catch.hpp"
#include "main/Application.h"
#include "main/Config.h"
#include "test/TestAccount.h"
#include "test/TestExceptions.h"
#include "test/TestUtils.h"
#include "test/TxTests.h"
#include "test/test.h"
#include "transactions/InflationOpFrame.h"
#include "transactions/OperationFrame.h"
#include "util/Logging.h"
#include "util/Timer.h"
#include <unistd.h>

using namespace stellar;
using namespace stellar::txtest;

typedef std::unique_ptr<Application> appPtr;

TEST_CASE("commonBudgetInflation","[CommonBudgetInflation]")
{
    std::string cfgFile("docs/tokennet_single_node_test.cfg");
    Config cfg;
    cfg.load(cfgFile);

    VirtualClock::time_point inflationStart;
    // inflation starts on 1-jul-2014
    time_t start = getTestDate(1, 7, 2014);
    inflationStart = VirtualClock::from_time_t(start);
    
    VirtualClock clock;
    clock.setCurrentTime(inflationStart);
    
    ApplicationEditableVersion app(clock, cfg);

    auto root = TestAccount::createRoot(app);

    app.start();
    auto& lm = app.getLedgerManager();
    int64_t setupBalance = lm.getMinBalance(0);
    auto ac = root.create("Common", 100000);

    SECTION("not time")
    {
          for_all_versions(app, [&] {
              closeLedgerOn(app, 2, 30, 6, 2014);
              REQUIRE_THROWS_AS(root.inflation(), ex_INFLATION_NOT_TIME);
  
              REQUIRE(
                  app.getLedgerManager().getCurrentLedgerHeader().inflationSeq ==
                  0);
  
              closeLedgerOn(app, 3, 1, 7, 2014);
  
              auto txFrame = root.tx({inflation()});
  
              closeLedgerOn(app, 4, 7, 7, 2014, {txFrame});
              REQUIRE(
                  app.getLedgerManager().getCurrentLedgerHeader().inflationSeq ==
                  1);
  
              REQUIRE_THROWS_AS(root.inflation(), ex_INFLATION_NOT_TIME);
              REQUIRE(
                  app.getLedgerManager().getCurrentLedgerHeader().inflationSeq ==
                  1);
  
              closeLedgerOn(app, 5, 8, 7, 2014);
              root.inflation();
              REQUIRE(
                  app.getLedgerManager().getCurrentLedgerHeader().inflationSeq ==
                  2);
      
              closeLedgerOn(app, 6, 14, 7, 2014);
              REQUIRE_THROWS_AS(root.inflation(), ex_INFLATION_NOT_TIME);
              REQUIRE(
                  app.getLedgerManager().getCurrentLedgerHeader().inflationSeq ==
                  2);

              closeLedgerOn(app, 7, 15, 7, 2014);
              root.inflation();
              REQUIRE(
                  app.getLedgerManager().getCurrentLedgerHeader().inflationSeq ==
                  3);

              closeLedgerOn(app, 8, 21, 7, 2014);
              REQUIRE_THROWS_AS(root.inflation(), ex_INFLATION_NOT_TIME);
              REQUIRE(
                  app.getLedgerManager().getCurrentLedgerHeader().inflationSeq ==
                  3);
        });
    }

    SECTION("total coins")
    {
        auto clh = app.getLedgerManager().getCurrentLedgerHeader();
        REQUIRE(clh.feePool == cfg.BASE_FEE);
        REQUIRE(clh.totalCoins == cfg.BALANCE);

        auto a1 = TestAccount{app, getAccount("a1"), 0};
        auto a2 = TestAccount{app, getAccount("a2"), 0};
        auto a3 = TestAccount{app, getAccount("a3"), 0};
        auto a4 = TestAccount{app, getAccount("a4"), 0};

        auto minBalance = app.getLedgerManager().getMinBalance(0);
        auto rootBalance = root.getBalance();

        auto a1tx = root.tx({createAccount(a1, 400000000000)});
        auto a2tx = root.tx({createAccount(a2, 300000000000)});
        auto a3tx = root.tx({createAccount(a3, 200000000000)});
        auto a4tx = root.tx({createAccount(a4, 100000000000)});

        closeLedgerOn(app, 2, 21, 7, 2014,
                      {a1tx, a2tx, a3tx, a4tx});

        clh = app.getLedgerManager().getCurrentLedgerHeader();

        REQUIRE(clh.feePool == cfg.BASE_FEE * 5);
        REQUIRE(clh.totalCoins == cfg.BALANCE);

        auto beforeInflationRoot = root.getBalance();
        auto beforeInflationCommon = ac.getBalance();
        auto beforeInflationA1 = a1.getBalance();
        auto beforeInflationA2 = a2.getBalance();
        auto beforeInflationA3 = a3.getBalance();
        auto beforeInflationA4 = a4.getBalance();

        REQUIRE(beforeInflationRoot + beforeInflationCommon + 
                    beforeInflationA1 + beforeInflationA2 + 
                    beforeInflationA3 + beforeInflationA4 + 
                    clh.feePool == clh.totalCoins);

        auto inflationTx = root.tx({inflation()});

        for_versions_from(8, app, [&] {
            closeLedgerOn(app, 3, 21, 7, 2014, {inflationTx});

            clh = app.getLedgerManager().getCurrentLedgerHeader();
            REQUIRE(clh.feePool == 0);
            REQUIRE(clh.totalCoins == cfg.BALANCE);

            auto afterInflationRoot = root.getBalance();
            auto afterInflationCommon = ac.getBalance();
            std::cout << afterInflationCommon << std::endl;
            auto afterInflationA1 = a1.getBalance();
            auto afterInflationA2 = a2.getBalance();
            auto afterInflationA3 = a3.getBalance();
            auto afterInflationA4 = a4.getBalance();

            REQUIRE(beforeInflationRoot == afterInflationRoot + cfg.BASE_FEE);
            REQUIRE(beforeInflationCommon == 
                afterInflationCommon - (cfg.BASE_FEE * 6) * 3 / 10);
            REQUIRE(beforeInflationA1 == 
                afterInflationA1 - (cfg.BASE_FEE * 6) * 7 / 10 * 4 / 10);
            REQUIRE(beforeInflationA2 == 
                afterInflationA2 - (cfg.BASE_FEE * 6) * 7 / 10 * 3 / 10);
            REQUIRE(beforeInflationA3 == 
                afterInflationA3 - (cfg.BASE_FEE * 6) * 7 / 10 * 2 / 10);
            REQUIRE(beforeInflationA4 == 
                afterInflationA4 - (cfg.BASE_FEE * 6) * 7 / 10 * 1 / 10);

            REQUIRE(afterInflationRoot + afterInflationCommon + 
                        afterInflationA1 + afterInflationA2 + 
                        afterInflationA3 + afterInflationA4 + 
                        clh.feePool == clh.totalCoins);
        });
    }
}
