#include <gtest/gtest-printers.h>
#include <gtest/gtest.h>

#include <game/mapitems.h>

namespace testing::internal {

template<>
class UniversalPrinter<CFixedTime>
{
public:
	static void Print(const CFixedTime &FixedTime, std::ostream *pOutputStream)
	{
		*pOutputStream << "CFixedTime with internal value " << FixedTime.GetInternal();
	}
};

} // namespace testing::internal

TEST(Mapitems, FixedTimeRoundtrip)
{
	for(CFixedTime Fixed = CFixedTime(0); Fixed < CFixedTime(1000000); Fixed += CFixedTime(1))
	{
		ASSERT_EQ(Fixed, CFixedTime::FromSeconds(Fixed.AsSeconds()));
	}
}
