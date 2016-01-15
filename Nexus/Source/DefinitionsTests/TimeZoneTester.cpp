#include "Nexus/DefinitionsTests/TimeZoneTester.hpp"
#include "Nexus/Definitions/DefaultMarketDatabase.hpp"
#include "Nexus/Definitions/Market.hpp"

using namespace boost;
using namespace boost::gregorian;
using namespace boost::posix_time;
using namespace Nexus;
using namespace Nexus::Tests;
using namespace std;

namespace {
  boost::local_time::tz_database GetTimeZoneDatabase() {
    stringstream ss;
    ss <<
      "\"Australian_Eastern_Standard_Time\",\"AEST\",\"AEST\",\"AEST\",\"AEST\",\"+10:00:00\",\"+01:00:00\",\"1;0;10\",\"+02:00:00\",\"1;0;4\",\"+03:00:00\"\n"
      "\"Eastern_Time\",\"EST\",\"Eastern Standard Time\",\"EDT\",\"Eastern Daylight Time\",\"-05:00:00\",\"+01:00:00\",\"2;0;3\",\"+02:00:00\",\"1;0;11\",\"+02:00:00\"\n"
      "\"UTC\",\"UTC\",\"UTC\",\"\",\"\",\"+00:00:00\",\"+00:00:00\",\"\",\"\",\"\",\"+00:00:00\"\n";
    boost::local_time::tz_database database;
    database.load_from_stream(ss);
    return database;
  }
}

void TimeZoneTester::TestTsxStartOfDay() {
  auto baseTime = ptime(date(1984, 5, 7), time_duration(1, 30, 0, 0));
  auto convertedTime = MarketDateToUtc(DefaultMarkets::TSX(), baseTime,
    GetDefaultMarketDatabase(), GetTimeZoneDatabase());
  auto expectedTime = ptime(date(1984, 5, 6), time_duration(4, 0, 0, 0));
  CPPUNIT_ASSERT(convertedTime == expectedTime);
}
