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

TEST_CASE("firstTransaction","[FirstTransaction]")
{
    std::string cfgFile("docs/stellar-core_example2.cfg");
    Config cfg;
    cfg.load(cfgFile);

    VirtualClock clock;
    ApplicationEditableVersion app(clock, cfg);
    app.start();

    auto root = TestAccount::createRoot(app);

    int64_t baseFee = app.getLedgerManager().getTxFee();

    const int64_t paymentAmount = app.getLedgerManager().getMinBalance(0);
    auto a1 = root.create("A", paymentAmount);

    AccountFrame::pointer rootAccount, a1Account;
    rootAccount = loadAccount(root, app);
    a1Account = loadAccount(a1, app);

    REQUIRE(a1Account->getBalance() == paymentAmount);
    REQUIRE(rootAccount->getBalance() == (cfg.BALANCE - paymentAmount - baseFee));
}
