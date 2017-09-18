#include "history/CatchupManager.h"
#include "ledger/LedgerManager.h"
#include "main/Application.h"
#include "main/Config.h"
#include "test/TestAccount.h"
#include "test/TestExceptions.h"
#include "test/TestUtils.h"
#include "test/TxTests.h"
#include "test/TestAccount.h"
#include "util/Logging.h"
#include "util/Timer.h"

TEST_CASE("ledgerSetting","[tx][ledgerSetting]")
{
    Config const& cfg = getTestConfig();

    VirtualClock clock;
    ApplicationEditableVersion app(clock, cfg);
    app.start();
		  
    // set up world
    auto root = TestAccount::createRoot(app);
    Asset xlmCur;
	   
    int64_t txfee = app.getLedgerManager().getTxFee();

    std::cout << rootAccount->getBalance() << endl;
}
