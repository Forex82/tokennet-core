#include "ledger/LedgerManager.h"
#include "lib/catch.hpp"
#include "main/Application.h"
#include "main/Config.h"
#include "test/TestAccount.h"
#include "test/TestExceptions.h"
#include "test/TestUtils.h"
#include "test/TxTests.h"
#include "test/test.h"
#include "util/Logging.h"
#include "util/Timer.h"
#include <unistd.h>

using namespace stellar;
using namespace stellar::txtest;

typedef std::unique_ptr<Application> appPtr;

TEST_CASE("ledgerSetting","[ledgerSetting]")
{
    // Config const& cfg = getTestConfig();
    std::string cfgFile("docs/stellar-core_example2.cfg");
    Config cfg;
    cfg.load(cfgFile);

    VirtualClock clock;
    ApplicationEditableVersion app(clock, cfg);
    app.start();

    auto root = TestAccount::createRoot(app);

    int64_t balance = app.getLedgerManager().getCurrentLedgerHeader().totalCoins;
    int64_t baseFee = app.getLedgerManager().getTxFee();
    int64_t baseReserve = 
	app.getLedgerManager().getCurrentLedgerHeader().baseReserve;
    uint32_t maxTxSetSize = app.getLedgerManager().getMaxTxSetSize();

    REQUIRE(balance == cfg.BALANCE);
    REQUIRE(baseFee == cfg.BASE_FEE);
    REQUIRE(baseReserve == cfg.BASE_RESERVE);
    REQUIRE(maxTxSetSize == cfg.MAX_TX_SET_SIZE);

    const int64_t paymentAmount = 
        app.getLedgerManager().getMinBalance(0) + 10 * baseFee;
    auto a1 = root.create("A", paymentAmount);

    AccountFrame::pointer rootAccount, a1Account;
    rootAccount = loadAccount(root, app);
    a1Account = loadAccount(a1, app);

    REQUIRE(a1Account->getBalance() == paymentAmount);
    REQUIRE(rootAccount->getBalance() == (cfg.BALANCE - paymentAmount - baseFee));
}
