#include "Stream.hpp"
#include <gtest/gtest.h>

TEST(StreamTest, givenEmptyStream_whenAdd_thenNoExceptions) {
    Stream<int> intstream;
    int myval = 10;
    EXPECT_NO_THROW(intstream.add(myval));
}

TEST(StreamTest, givenStreamWithData_whenNext_thenReturnLastDataPut) {
    Stream<int> intstream;
    int myval = 10;
    EXPECT_TRUE(intstream.add(myval));
    auto next = intstream.next();
    EXPECT_TRUE(next.has_value());
    EXPECT_EQ(next.value(), myval);
}

TEST(StreamTest, givenEmptyStreamAfterEos_whenNext_thenEmptyOptional) {
    Stream<int> intstream;
    intstream.eos();
    EXPECT_FALSE(intstream.next().has_value());
}

TEST(StreamTest, givenStreamWithDataAfterEos_whenNext_thenOptionalWithLastData) {
    Stream<int> intstream;
    int myval = 40;
    EXPECT_TRUE(intstream.add(myval));
    intstream.eos();
    EXPECT_TRUE(intstream.next().has_value());
    EXPECT_FALSE(intstream.next().has_value());
}

TEST(StreamTest, givenStreamWithData_whenMultipleNext_thenReturnsCorrectDataOrder) {
    Stream<int> intstream;
    int myval1 = 10, myval2 = 20;
    EXPECT_TRUE(intstream.add(myval1));
    EXPECT_TRUE(intstream.add(myval2));
    intstream.eos();
    auto next1 = intstream.next();
    EXPECT_TRUE(next1.has_value());
    EXPECT_EQ(next1.value(), myval1);
    auto next2 = intstream.next();
    EXPECT_TRUE(next2.has_value());
    EXPECT_EQ(next2.value(), myval2);
    EXPECT_FALSE(intstream.next().has_value());
}

TEST(StreamTest, givenEOS_whenAdd_thenReturnsFalse) {
    Stream<int> intstream;
    int myval = 17;
    intstream.eos();
    EXPECT_FALSE(intstream.add(myval));
    EXPECT_FALSE(intstream.next().has_value());
}