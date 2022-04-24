#include "constraints.hpp"
#include <catch2/catch.hpp>

using namespace dlx;

TEST_CASE("matrix constructed 1", "[matrix]")
{
  SECTION("size 1")
  {
    AlgorithmX algo(1);
    SECTION("no clues")
    {
      std::list<AlgorithmX::SolutionKey> clues;
      auto [matrix, partial] = algo.create_matrix(clues);
      REQUIRE(matrix.size() == 1U);
    }
  }
}

TEST_CASE("matrix constructed 4", "[matrix]")
{
  SECTION("size 4")
  {
    AlgorithmX algo(4);
    SECTION("no clues")
    {
      std::list<AlgorithmX::SolutionKey> clues;
      auto [matrix, partial] = algo.create_matrix(clues);
      REQUIRE(matrix.size() == 64U);
    }
    SECTION("clues")
    {
      std::list<AlgorithmX::SolutionKey> clues;
      clues.emplace_back(1, 1, 1);
      auto [matrix, partial] = algo.create_matrix(clues);
      REQUIRE(matrix.size() < 64U);
      algo.solve(matrix, partial);
    }
  }
}

TEST_CASE("9", "[solve]")
{
  AlgorithmX algo(9);
  std::list<AlgorithmX::SolutionKey> clues;
  clues.emplace_back(1, 4, 2);
  clues.emplace_back(1, 5, 5);
  clues.emplace_back(1, 7, 7);
  clues.emplace_back(1, 9, 9);
  clues.emplace_back(2, 3, 9);
  clues.emplace_back(2, 5, 6);
  clues.emplace_back(2, 6, 3);
  clues.emplace_back(3, 1, 8);
  clues.emplace_back(3, 8, 2);
  clues.emplace_back(4, 2, 9);
  clues.emplace_back(4, 3, 8);
  clues.emplace_back(4, 5, 7);
  clues.emplace_back(4, 6, 4);
  clues.emplace_back(5, 1, 3);
  clues.emplace_back(5, 3, 5);
  clues.emplace_back(5, 5, 8);
  clues.emplace_back(5, 7, 2);
  clues.emplace_back(5, 9, 4);
  clues.emplace_back(6, 4, 6);
  clues.emplace_back(6, 5, 2);
  clues.emplace_back(6, 7, 3);
  clues.emplace_back(6, 8, 9);
  clues.emplace_back(7, 2, 8);
  clues.emplace_back(7, 9, 7);
  clues.emplace_back(8, 4, 1);
  clues.emplace_back(8, 5, 4);
  clues.emplace_back(8, 7, 8);
  clues.emplace_back(9, 1, 5);
  clues.emplace_back(9, 3, 7);
  clues.emplace_back(9, 6, 9);
  clues.emplace_back(9, 7, 6);
  auto [matrix, partial] = algo.create_matrix(clues);
  AlgorithmX::print_matrix(matrix);
  algo.solve(matrix, partial);
}
