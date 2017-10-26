#include "ledger/AccountFrame.h"
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
    std::string cfgFile("docs/tokennet_single_node_test.cfg");
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

    AccountID aid(KeyUtils::fromStrKey<PublicKey>(cfg.COMMON_BUDGET_ACCOUNT_ID));
    REQUIRE(KeyUtils::toStrKey(aid) == cfg.COMMON_BUDGET_ACCOUNT_ID);
}
