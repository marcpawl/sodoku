#pragma once

#include <array>
#include <cassert>
#include <cmath>
#include <cstdint>
#include <list>
#include <map>
#include <set>
#include <variant>
#include <vector>

namespace dlx {


struct AlgorithmX
{
  /** Placement of a digit in a cell. */
  struct SolutionKey
  {
    size_t const puzzle_row_;
    size_t const puzzle_column_;
    size_t const digit_;
    SolutionKey(const size_t puzzle_row, const size_t puzzle_column, const size_t digit);
    friend int operator<=>(const SolutionKey &lhs, const SolutionKey &rhs);
  };

  /** Only one digit per cell. */
  struct CellConstraintKey
  {
    size_t const puzzle_row_;
    size_t const puzzle_column_;
    CellConstraintKey(const size_t puzzle_row, const size_t puzzle_column)
      : puzzle_row_(puzzle_row), puzzle_column_(puzzle_column)
    {}
    friend bool operator<(const CellConstraintKey &lhs, const CellConstraintKey &rhs)
    {
      if (lhs.puzzle_row_ < rhs.puzzle_row_) return true;
      if (rhs.puzzle_row_ < lhs.puzzle_row_) return false;
      return lhs.puzzle_column_ < rhs.puzzle_column_;
    }
    bool operator==(const CellConstraintKey &rhs) const
    {
      return puzzle_row_ == rhs.puzzle_row_ && puzzle_column_ == rhs.puzzle_column_;
    }
    bool operator!=(const CellConstraintKey &rhs) const { return !(rhs == *this); }
  };

  /** Digit can only appear once in the row. */
  struct RowConstraintKey
  {
    size_t const puzzle_row_;
    size_t const digit_;
    RowConstraintKey(const size_t puzzle_row, const size_t digit) : puzzle_row_(puzzle_row), digit_(digit) {}
    friend bool operator<(const RowConstraintKey &lhs, const RowConstraintKey &rhs)
    {
      if (lhs.puzzle_row_ < rhs.puzzle_row_) return true;
      if (rhs.puzzle_row_ < lhs.puzzle_row_) return false;
      return lhs.digit_ < rhs.digit_;
    }
    bool operator==(const RowConstraintKey &rhs) const
    {
      return puzzle_row_ == rhs.puzzle_row_ && digit_ == rhs.digit_;
    }
    bool operator!=(const RowConstraintKey &rhs) const { return !(rhs == *this); }
  };

  /** Digit can only appear once in the column. */
  struct ColumnConstraintKey
  {
    size_t const puzzle_column_;
    size_t const digit_;
    ColumnConstraintKey(const size_t puzzle_column, const size_t digit) : puzzle_column_(puzzle_column), digit_(digit)
    {}
    friend bool operator<(const ColumnConstraintKey &lhs, const ColumnConstraintKey &rhs)
    {
      if (lhs.puzzle_column_ < rhs.puzzle_column_) return true;
      if (rhs.puzzle_column_ < lhs.puzzle_column_) return false;
      return lhs.digit_ < rhs.digit_;
    }
  };

  /** Digit can only appear once in a block. */
  struct BlockConstraintKey
  {
    size_t const puzzle_block_column_;
    size_t const puzzle_row_column_;
    size_t const digit_;
    BlockConstraintKey(const size_t puzzle_block_column, const size_t puzzle_row_column, const size_t digit)
      : puzzle_block_column_(puzzle_block_column), puzzle_row_column_(puzzle_row_column), digit_(digit)
    {}
    friend bool operator<(const BlockConstraintKey &lhs, const BlockConstraintKey &rhs)
    {
      if (lhs.puzzle_block_column_ < rhs.puzzle_block_column_) return true;
      if (rhs.puzzle_block_column_ < lhs.puzzle_block_column_) return false;
      if (lhs.puzzle_row_column_ < rhs.puzzle_row_column_) return true;
      if (rhs.puzzle_row_column_ < lhs.puzzle_row_column_) return false;
      return lhs.digit_ < rhs.digit_;
    }
    bool operator==(const BlockConstraintKey &rhs) const
    {
      return puzzle_block_column_ == rhs.puzzle_block_column_ && puzzle_row_column_ == rhs.puzzle_row_column_
             && digit_ == rhs.digit_;
    }
    bool operator!=(const BlockConstraintKey &rhs) const { return !(rhs == *this); }
  };

  using ConstraintKey = std::variant<CellConstraintKey, RowConstraintKey, ColumnConstraintKey, BlockConstraintKey>;

  using Row = std::set<ConstraintKey>;
  using Matrix = std::map<SolutionKey, Row>;

  /** Create a matrix with all the constraints being set to false. */
  Matrix create_empty_matrix() const;

  /** Create a matrix with the constraints set. */
  Matrix populate_entry_matrix() const;


  /** Remove the rows that conflict with the clue. */
  void constrain(Matrix &matrix, SolutionKey const &clue) const;


  /** Create the matrix with the constraints set, removing possible solutions that are
   *  disallowed by the clues.
   * @param clues Description of the puzzle to solve.
   * @return Possible solution matrix with the constraints, and the clues as the partial solution set.
   */
  std::tuple<AlgorithmX::Matrix, std::set<AlgorithmX::SolutionKey>> create_matrix(
    std::list<SolutionKey> const &clues) const;

  static ConstraintKey choose_column(Matrix const &matrix);

  void solve(Matrix A, std::set<AlgorithmX::SolutionKey> partial_solution);


  void print_solution(std::set<AlgorithmX::SolutionKey> const &solution) const;
  static void print_matrix(Matrix const& matrix);

  size_t const puzzle_size_;

  explicit AlgorithmX(const size_t puzzle_size);
  static void delete_column(Matrix &A, std::set<ConstraintKey > const& constraints);
  static void delete_row(Matrix &A, std::set< ConstraintKey> const& constraints);
   void cover(Matrix &A, const SolutionKey &r, std::set<AlgorithmX::SolutionKey> &partial_solution) const;

  // Check that the working solution is meeting the algorithm constraings
  void check_is_valid(std::set<AlgorithmX::SolutionKey> &partial_solution, AlgorithmX::Matrix &A) const;
};

}// namespace dlx
