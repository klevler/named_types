#include <iostream>
#include <gtest/gtest.h>
#include <string>
#include <vector>
#include <tuple>
#include <array>
#include <functional>
#include <named_types/named_tuple.hpp>
#include <named_types/literals/integral_string_literal.hpp>
#include <named_types/rt_named_tuple.hpp>
#include <named_types/extensions/factory.hpp>

using namespace named_types;

namespace {
struct name {};
struct age {
  inline static char const* name() { return "age4"; }
};
struct taille;
struct size;
struct liste;
struct func;
struct birthday; // Not used, for failure tests
struct surname;
}

class UnitTests : public ::testing::Test {
 protected:
  named_tuple<std::string(name),
              int(age),
              double(taille),
              std::vector<int>(liste),
              std::function<int(int)>(func)> test{
      "Roger", 47, 1.92, {1, 2, 3}, [](int a) { return a * a; }};
};

static named_tag<name> name_key;

TEST_F(UnitTests, Construction1) {
  EXPECT_EQ(std::string("Roger"), std::get<0>(test));
  EXPECT_EQ(std::string("Roger"), std::get<named_tag<name>>(test));
  EXPECT_EQ(std::string("Roger"), named_types::get<name>(test));
  EXPECT_EQ(std::string("Roger"), named_types::get<named_tag<name>>(test));
  EXPECT_EQ(std::string("Roger"), test.get<name>());
  EXPECT_EQ(std::string("Roger"), test.get<named_tag<name>>());
  EXPECT_EQ(std::string("Roger"), test[name_key]);
  EXPECT_EQ(std::string("Roger"), name_key(test));
}

TEST_F(UnitTests, Traits1) {
  using TupleType = decltype(test);
  EXPECT_TRUE(TupleType::has_tag<named_tag<name>>::value);
  EXPECT_FALSE(TupleType::has_tag<named_tag<birthday>>::value);
}

TEST_F(UnitTests, MakeTuple1) {
  auto t = make_named_tuple(name_key = std::string("Roger"));
  EXPECT_EQ(std::string("Roger"), t[name_key]);
}

TEST_F(UnitTests, Promotion1) {
  named_tag<name> name_k;
  named_tag<size> size_k;
  named_tag<surname> surname_k;
  named_tag<birthday> bday_k;

  auto t1 = make_named_tuple(name_k = std::string("Roger"),
                             surname_k = std::string("LeGros"),
                             size_k = 3u);

  auto t2 = make_named_tuple(
      size_k = 4u, name_k = std::string("Marcel"), bday_k = 1e6);

  decltype(t1) t3(t2); // Copy
  EXPECT_EQ(std::string("Marcel"), t3[name_k]);
  EXPECT_EQ(4u, t3[size_k]);

  decltype(t1) t4(std::move(t2)); // Move
  EXPECT_EQ(std::string("Marcel"), t4[name_k]);
  EXPECT_NE(std::string("Marcel"), t2[name_k]);
  EXPECT_EQ(4u, t4[size_k]);
}

TEST_F(UnitTests, Injection1) {
  named_tag<name> name_k;
  named_tag<size> size_k;
  named_tag<surname> surname_k;
  named_tag<birthday> bday_k;

  auto t1 = make_named_tuple(name_k = std::string("Roger"),
                             surname_k = std::string("LeGros"),
                             size_k = 3u);

  auto t2 = make_named_tuple(
      size_k = 4u, name_k = std::string("Marcel"), bday_k = 1e6);

  t1 = t2; // Copy
  EXPECT_EQ(std::string("Marcel"), t1[name_k]);
  EXPECT_EQ(std::string("LeGros"), t1[surname_k]);
  EXPECT_EQ(4u, t1[size_k]);

  t1[name_k] = "Robert";
  EXPECT_EQ(std::string("Robert"), t1[name_k]);

  t2 = std::move(t1); // Move
  EXPECT_EQ(std::string("Robert"), t2[name_k]);
  EXPECT_NE(std::string("Robert"), t1[name_k]);
  EXPECT_EQ(std::string("LeGros"), t1[surname_k]);
  EXPECT_EQ(4u, t1[size_k]);
}

TEST_F(UnitTests, TaggedTuple) { struct name : std::tag::basic_tag { };
  struct size : std::tag::basic_tag {};
  struct yo : std::tag::basic_tag {};
  using T1 = std::tagged_tuple<name(std::string), size(size_t), yo(name)>;
  T1 t{"Roger", 3, {}};

  EXPECT_EQ(0, T1::tag_index<name>::value);
  EXPECT_EQ(std::string("Roger"), std::get<T1::tag_index<name>::value>(t));
  EXPECT_EQ(std::string("Roger"), std::get<name>(t));

  EXPECT_EQ(1, T1::tag_index<size>::value);
  EXPECT_EQ(3, std::get<T1::tag_index<size>::value>(t));
  EXPECT_EQ(3, std::get<size>(t));

  auto t2 = t;
  EXPECT_EQ(3, std::get<size>(t2));
}

TEST_F(UnitTests, ConstexprStrings1) {
  EXPECT_EQ(193488139,
            (std::integral_constant<
                size_t,
                string_literal<char, 'a', 'b', 'c'>::hash_value>::value));
  EXPECT_EQ(193488139,
            (std::integral_constant<size_t, const_hash("abc")>::value));
  EXPECT_EQ(
      1u,
      (std::integral_constant<size_t, arithmetic::pow<size_t>(2, 0)>::value));
  EXPECT_EQ(
      2u,
      (std::integral_constant<size_t, arithmetic::pow<size_t>(2, 1)>::value));
  EXPECT_EQ(
      4u,
      (std::integral_constant<size_t, arithmetic::pow<size_t>(2, 2)>::value));

  EXPECT_EQ(
      31u,
      (std::integral_constant<size_t,
                              integral_string_format<uint32_t, char, 'a', 'b'>::
                                  max_length_value>::value));
  EXPECT_EQ(12u,
            (std::integral_constant<
                size_t,
                basic_lowcase_charset_format::max_length_value>::value));
  EXPECT_EQ(
      10u,
      (std::integral_constant<size_t,
                              basic_charset_format::max_length_value>::value));
  EXPECT_EQ(
      8u,
      (std::integral_constant<size_t,
                              ascii_charset_format::max_length_value>::value));

  constexpr uint64_t const str_test1 =
      std::integral_constant<uint64_t,
                             basic_charset_format::encode("coucou")>::value;
  EXPECT_EQ(
      6u,
      (std::integral_constant<
          size_t,
          basic_charset_format::decode<str_test1>::type().size()>::value));
  EXPECT_EQ(std::string("coucou"),
            std::string(basic_charset_format::decode<str_test1>::type().str()));

  constexpr uint64_t const str_test2 = std::integral_constant<
      uint64_t,
      basic_lowcase_charset_format::encode("aaaaaaaaaaaa")>::value;
  EXPECT_EQ(12u,
            (std::integral_constant<size_t,
                                    basic_lowcase_charset_format::decode<
                                        str_test2>::type().size()>::value));
  EXPECT_EQ(std::string("aaaaaaaaaaaa"),
            std::string(
                basic_lowcase_charset_format::decode<str_test2>::type().str()));

  constexpr uint64_t const str_test3 = std::integral_constant<
      uint64_t,
      basic_lowcase_charset_format::encode("------------")>::value;
  EXPECT_EQ(12u,
            (std::integral_constant<size_t,
                                    basic_lowcase_charset_format::decode<
                                        str_test3>::type().size()>::value));
  EXPECT_EQ(std::string("------------"),
            std::string(
                basic_lowcase_charset_format::decode<str_test3>::type().str()));

  constexpr uint64_t const str_test4 =
      std::integral_constant<uint64_t,
                             basic_lowcase_charset_format::encode("")>::value;
  EXPECT_EQ(0u,
            (std::integral_constant<size_t,
                                    basic_lowcase_charset_format::decode<
                                        str_test4>::type().size()>::value));
  EXPECT_EQ(std::string(""),
            std::string(
                basic_lowcase_charset_format::decode<str_test4>::type().str()));

  EXPECT_EQ(
      std::string("abcdefg"),
      std::string(concatenate<string_literal<char, 'a', 'b', 'c'>,
                              string_literal<char, 'd', 'e'>,
                              string_literal<char, 'f', 'g'>>::type().str()));
}

TEST_F(UnitTests, ConstexprStrings2) {
  using Str1 = string_literal<char,'R','o','g','e','r'>;
  using Str2 = string_literal<char,'M','a','r','c','e','l'>;
  using Str3 = string_literal<char,'P','a','s','t','i','s'>;

  EXPECT_EQ(std::string("Roger"), (join<char,',',Str1>::type::data));
  EXPECT_EQ(std::string("Roger,Marcel"), (join<char,',',Str1,Str2>::type::data));
  EXPECT_EQ(std::string("Roger,Marcel,Pastis"), (join<char,',',Str1,Str2,Str3>::type::data));
}

size_t constexpr operator"" _s(const char* c, size_t s) {
  return named_types::basic_lowcase_charset_format::encode(c, s);
}
template <size_t EncStr>
using attr = named_tag<
    typename named_types::basic_lowcase_charset_format::decode<EncStr>::type>;

TEST_F(UnitTests, RuntimeView1) {
  attr<"name"_s> name_k;
  attr<"size"_s> size_k;
  attr<"surname"_s> surname_k;
  attr<"birthday"_s> bday_k;

  auto t1 = make_named_tuple(name_k = std::string("Roger"),
                             surname_k = std::string("LeGros"),
                             size_k = 3u);

  decltype(t1) const& t1_const = t1;

  auto const_view1 = make_rt_view(t1_const);
  //std::cout << *const_view1.retrieve<std::string>("name") << std::endl;
}
