#include <gtest/gtest.h>
#include "primitives.h"

#include <iostream>
#include <fstream>

template <typename T>
class PointSetTest : public ::testing::Test {
public:
	void load_data(const std::string & filename)
	{
		try {
			std::ifstream fs(filename);
			ASSERT_TRUE(fs.is_open()) << filename << " wasn't opened";

			double x, y;
			while (fs) {
				fs >> x >> y;
                if (fs.fail()) {
                    break;
                }
				m_set.put(Point(x, y));
			}

			// std::cout << "Loaded " << m_set.size() << " points.\n";
		} catch(...) {
			std::cout << "Can't read " << filename << ".\n";
		}
    }

    void check_size(size_t correct_size)
    {
        auto it = m_set.begin();
        auto end = m_set.end();
        ASSERT_TRUE(it != end);
        size_t s = 0;
        while (it != end) {
            ++s;
            ++it;
        }
        ASSERT_EQ(s, correct_size);
    }

	T m_set;
};

using TestTypes = ::testing::Types<rbtree::PointSet, kdtree::PointSet>;
TYPED_TEST_SUITE(PointSetTest, TestTypes);


TEST(PointSetTest, Point)
{
	ASSERT_TRUE(Point(1., 2.) == Point(1., 2.));
	ASSERT_TRUE(Point(1., 2.) != Point(5., 4.));
	ASSERT_EQ(Point(0, 0).distance(Point(1, 0)), 1.);
	ASSERT_EQ(Point(0, 0).distance(Point(0, 1)), 1.);
	ASSERT_EQ(Point(0, 4).distance(Point(3, 0)), 5.);
}

TEST(PointSetTest, Rect)
{
	Rect r(Point(1., 1.), Point(2., 2.));
	ASSERT_EQ(r.distance(Point(1., 1.)), 0.);
	ASSERT_EQ(r.distance(Point(1.5, 1.5)), 0.);
	ASSERT_EQ(r.distance(Point(0., 1.)), 1.);
	ASSERT_TRUE(r.contains(Point(1.5, 1.5)));
	ASSERT_FALSE(r.contains(Point(.9, 1.5)));
	ASSERT_TRUE(r.intersects(Rect(Point(0., 0.), Point(1.5, 1.5))));
	ASSERT_TRUE(r.intersects(Rect(Point(0.5, 0.5), Point(3.5, 3.5))));
	ASSERT_FALSE(r.intersects(Rect(Point(2.1, 0.1), Point(3.5, 1.9))));
}

TYPED_TEST(PointSetTest, PointSetMethods)
{
	auto & p = this->m_set;
	ASSERT_TRUE(p.empty());
	ASSERT_EQ(p.size(), 0);

	p.put(Point(0, 0));
	p.put(Point(1., 1.));
	p.put(Point(.5, .5));
	ASSERT_FALSE(p.empty());
	ASSERT_EQ(p.size(), 3);
    this->check_size(3);
	ASSERT_TRUE(p.contains(Point(0,0)));
	ASSERT_FALSE(p.contains(Point(0.5,0)));

    auto it = p.begin();
	ASSERT_TRUE((it->x() == 0.) && (it->y() == 0.));
}

TYPED_TEST(PointSetTest, PointSetBasicSearch)
{
	auto & p = this->m_set;
	p.put(Point(0, 0));
	p.put(Point(1., 1.));
	p.put(Point(.5, .5));
    this->check_size(3);
	
	auto n = p.nearest(Point(.4, .4));
	ASSERT_TRUE(n.has_value());
	ASSERT_TRUE(Point(.5, .5) == *n);

	auto range = p.range(Rect(Point(0.3, 0.3), Point(.7, .7)));
	auto & it = range.first;
	auto & out_of_range = range.second;
	ASSERT_TRUE(*it== Point(.5, .5));
    ++it;
	ASSERT_TRUE(it == out_of_range);
}

TYPED_TEST(PointSetTest, PointSetNearest0)
{
	this->load_data("test/etc/test0.dat");
	auto & p = this->m_set;
    this->check_size(5);
	
	auto n = p.nearest(Point(.74, .29));
	ASSERT_TRUE(n.has_value());
	ASSERT_TRUE(Point(0.725, 0.338) == *n);
    this->check_size(5);
}

TYPED_TEST(PointSetTest, PointSetNearest1)
{
    this->load_data("test/etc/test2.dat");
    auto & p = this->m_set;
    this->check_size(120);

    auto n = p.nearest(Point(.712, .567));
	ASSERT_TRUE(n.has_value());
	ASSERT_TRUE(Point(0.718, 0.555) == *n);
    this->check_size(120);
}

TYPED_TEST(PointSetTest, PointSetRange0)
{
    this->load_data("test/etc/test1.dat");
    auto & p = this->m_set;
    this->check_size(20);

    auto range = p.range(Rect(Point(0.634, 0.276), Point(.818, .42)));

    auto & it = range.first;
    auto & out_of_range = range.second;

	ASSERT_TRUE(it != out_of_range);
	ASSERT_TRUE(*it == Point(0.655, 0.382));
	++it;
	ASSERT_TRUE(*it == Point(0.725, 0.311));
	++it;
	ASSERT_TRUE(*it == Point(0.794, 0.299));
	++it;
	ASSERT_TRUE(it == out_of_range); 
    this->check_size(20);
}

TYPED_TEST(PointSetTest, PointSetNearestK1)
{
    this->load_data("test/etc/test2.dat");
    auto & p = this->m_set;
    this->check_size(120);

	auto range = p.nearest(Point(.386, .759), 3);
    auto & it = range.first;
    auto & out_of_range = range.second;

	ASSERT_TRUE(it != out_of_range);
	ASSERT_TRUE(*it == Point(0.376, 0.767));
	++it;
	ASSERT_TRUE(*it == Point(0.409, 0.754));
	++it;
	ASSERT_TRUE(*it == Point(0.408, 0.728));
	++it;
	ASSERT_TRUE(it == out_of_range); 
    this->check_size(120);
}
