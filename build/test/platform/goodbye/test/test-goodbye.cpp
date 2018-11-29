#include "../goodbye.cpp"

#include <iostream>

#include <gtest/gtest.h>

using namespace std;

// Fixture-less tests

TEST(Goodbye, Fatal)
{
    ASSERT_EQ( "Goodbye", NCBI::VDB3::GoodbyeMsg() );
}

TEST(Goodbye, nonFatal)
{
    // uncomment the next line if you want to see non-fatal asseretions in action
    // EXPECT_TRUE(false); // reoports non-fatal failure and continues the function
    ASSERT_EQ( "Goodbye", NCBI::VDB3::GoodbyeMsg() );
}

// Tests with fixtures

class Fixture : public ::testing::Test
{
 protected:
    Fixture()
    : m_value ( 0 )
    {
    }

    void SetUp() override
    {
        m_value = 1;
    }
    void TearDown() override
    {
        m_value = 2;
    }

    int m_value;
};

TEST_F(Fixture, Simple)
{
    EXPECT_EQ( 1, m_value );
}

// main

int main(int argc, char **argv)
{
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
