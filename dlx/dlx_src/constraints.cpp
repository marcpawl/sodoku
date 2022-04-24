
#include "constraints.hpp"
#include "spdlog/spdlog.h"
#include <cstdio>
#include <execution>
#include <iostream>
#include <numeric>
#include <optional>
#include <exception>

namespace dlx {


int operator<=>(const AlgorithmX::SolutionKey &lhs, const AlgorithmX::SolutionKey &rhs)
{
  if (lhs.puzzle_row_ < rhs.puzzle_row_) { return -1; }
  if (rhs.puzzle_row_ < lhs.puzzle_row_) { return 1; }
  if (lhs.puzzle_column_ < rhs.puzzle_column_) { return -1; }
  if (rhs.puzzle_column_ < lhs.puzzle_column_) { return 1; }
  if (lhs.digit_ < rhs.digit_) { return -1; }
  if (rhs.digit_ < lhs.digit_) { return 1; }
  return 0;
}
AlgorithmX::SolutionKey::SolutionKey(const size_t puzzle_row, const size_t puzzle_column, const size_t digit)// NOLINT
  : puzzle_row_(puzzle_row), puzzle_column_(puzzle_column), digit_(digit)
{}


AlgorithmX::AlgorithmX(const size_t puzzle_size) : puzzle_size_(puzzle_size) {}

AlgorithmX::Matrix AlgorithmX::create_empty_matrix() const// NOLINT
{
  auto const nb_blocks = static_cast<size_t>(std::sqrt(puzzle_size_));
  assert(nb_blocks * nb_blocks == puzzle_size_);// NOLINT

  Matrix result = Matrix();
  for (size_t puzzle_row = 1; puzzle_row <= puzzle_size_; ++puzzle_row) {
    for (size_t puzzle_column = 1; puzzle_column <= puzzle_size_; ++puzzle_column) {
      for (size_t digit = 1; digit <= puzzle_size_; ++digit) {
        SolutionKey solution_key(puzzle_row, puzzle_column, digit);
        result.insert(std::pair(solution_key, Row()));
        Row &row = result[solution_key];
      }
    }
  }
  return result;
}

AlgorithmX::Matrix AlgorithmX::populate_entry_matrix() const
{
  auto const nb_blocks = static_cast<size_t>(std::sqrt(puzzle_size_));
  assert(nb_blocks * nb_blocks == puzzle_size_);// NOLINT

  Matrix matrix = create_empty_matrix();
  for (size_t puzzle_row = 1; puzzle_row <= puzzle_size_; ++puzzle_row) {
    for (size_t puzzle_column = 1; puzzle_column <= puzzle_size_; ++puzzle_column) {
      for (size_t digit = 1; digit <= puzzle_size_; ++digit) {
        SolutionKey solution_key(puzzle_row, puzzle_column, digit);
        Row &row = matrix.at(solution_key);
        CellConstraintKey cell_constraint_key(puzzle_row, puzzle_column);
        row.insert(cell_constraint_key);
        RowConstraintKey row_constraint_key(puzzle_row, digit);
        row.insert(row_constraint_key);
        ColumnConstraintKey column_constraint_key(puzzle_column, digit);
        row.insert(column_constraint_key);
        size_t block_row = ((puzzle_row - 1) / nb_blocks) + 1;
        size_t block_col = ((puzzle_column - 1) / nb_blocks) + 1;
        BlockConstraintKey block_constraint_key(block_row, block_col, digit);
        row.insert(block_constraint_key);
      }
    }
  }
  return matrix;
}

void AlgorithmX::constrain(Matrix &matrix, SolutionKey const &clue) const
{
  for (size_t puzzle_row = 1; puzzle_row <= puzzle_size_; ++puzzle_row) {
    for (size_t puzzle_column = 1; puzzle_column <= puzzle_size_; ++puzzle_column) {
      for (size_t digit = 1; digit <= puzzle_size_; ++digit) {
        if (clue.puzzle_row_ == puzzle_row && clue.puzzle_column_ == puzzle_column && digit != clue.digit_) {
          SolutionKey solution_key(puzzle_row, puzzle_column, digit);
          matrix.erase(solution_key);
        }
      }
    }
  }
}

std::tuple<AlgorithmX::Matrix, std::set<AlgorithmX::SolutionKey>> AlgorithmX::create_matrix(
  std::list<SolutionKey> const &clues) const
{
  std::set<AlgorithmX::SolutionKey> partial_solution;
  Matrix matrix = populate_entry_matrix();
  for (SolutionKey const &clue : clues) {
    constrain(matrix, clue);
    cover(matrix, clue, partial_solution);
  }
  return { matrix, partial_solution };
}

int count_use(AlgorithmX::Matrix const &matrix, AlgorithmX::ConstraintKey const &constraint)
{
  auto is_true = [&constraint](int count, auto const &row_ptr) -> int {
    AlgorithmX::Row const &row = row_ptr.second;
    if (row.contains(constraint)) { return count + 1; }
    return count;
  };
  return std::reduce(std::execution::par_unseq, std::begin(matrix), std::end(matrix), 0, is_true);
}

AlgorithmX::ConstraintKey AlgorithmX::choose_column(Matrix const &matrix)
{
  auto first_row = std::begin(matrix);
  std::vector<std::tuple<ConstraintKey, int>> constraints;
  constraints.reserve(first_row->second.size());
  std::transform(std::begin(first_row->second),
    std::end(first_row->second),
    std::back_inserter(constraints),
    [&matrix](ConstraintKey const &constraint) -> std::tuple<ConstraintKey, int> {
      return { constraint, count_use(matrix, constraint) };
    });

  auto max_constraint = std::max_element(std::execution::par_unseq,
    std::begin(constraints),
    std::end(constraints),
    [](auto const &lhs, auto const &rhs) -> bool { return std::get<int>(lhs) < std::get<int>(rhs); });
  return std::get<ConstraintKey>(*max_constraint);
}

std::optional<AlgorithmX::SolutionKey> choose_row(AlgorithmX::Matrix const &A, AlgorithmX::ConstraintKey const &c)
{
  auto iter = std::find_if(
    std::begin(A), std::end(A), [&c](auto const &row_pair) -> bool { return row_pair.second.contains(c); });
  if (iter == std::end(A)) { return std::nullopt; }
  return iter->first;
}

void AlgorithmX::print_matrix(Matrix const &matrix)
{
  spdlog::critical("matrix");
  for (auto const &row_pair : matrix) {
    SolutionKey const &key = row_pair.first;
    spdlog::critical("{} {} {}", key.puzzle_row_, key.puzzle_column_, key.digit_);
  }
}

void AlgorithmX::print_solution(std::set<AlgorithmX::SolutionKey> const &solution) const
{
  std::vector<std::vector<char>> m;
  m.reserve(this->puzzle_size_);
  for (int i = 0; i < this->puzzle_size_; ++i) { m.emplace_back(std::vector<char>(puzzle_size_, '.')); }
  for (SolutionKey const &key : solution) {
    // TODO make it base the puzzle size
    m.at(key.puzzle_row_ - 1).at(key.puzzle_column_ - 1) = std::to_string(key.digit_)[0];
    spdlog::critical("{} {} -> {}", key.puzzle_row_, key.puzzle_column_, key.digit_);
  }
  auto box_size = static_cast<size_t>(std::sqrt(puzzle_size_));
  for (size_t puzzle_row = 0; puzzle_row < puzzle_size_; ++puzzle_row) {
    if (puzzle_row != 0 && ((puzzle_row % box_size) == 0)) {
      for (size_t puzzle_column = 0; puzzle_column < puzzle_size_; ++puzzle_column) { std::clog << '-'; }
      std::clog << '\n';
    }
    for (size_t puzzle_column = 0; puzzle_column < puzzle_size_; ++puzzle_column) {
      if (puzzle_column != 0 && ((puzzle_column % box_size) == 0)) { std::clog << '|'; }
      std::clog << m.at(puzzle_row).at(puzzle_column);
    }
    std::clog << '\n';
  }
}

void AlgorithmX::solve(Matrix A, std::set<AlgorithmX::SolutionKey> partial_solution)// NOLINT(misc-no-recursion)
{
  if (A.empty()) {
    spdlog::critical("SUCCESS");
    print_solution(partial_solution);
    return;
  }

  //    Otherwise choose a column, c (deterministically).
  ConstraintKey c = choose_column(A);
  //                               Choose a row, r, such that A[r, c] = 1 (nondeterministically).
  std::optional<SolutionKey> r_opt = choose_row(A, c);
  if (!r_opt.has_value()) {
    // failure.
    return;
  }
  SolutionKey const &r = r_opt.value();
  //                             Include r in the partial solution.
  cover(A, r, partial_solution);
  //   Repeat this algorithm recursively on the reduced matrix A.
  solve(A, partial_solution);
}

void AlgorithmX::check_is_valid(std::set<AlgorithmX::SolutionKey> &partial_solution, AlgorithmX::Matrix &A) const
{
  std::vector<std::vector<bool>> has_cell(this->puzzle_size_, std::vector<bool>(this->puzzle_size_, false));
  for (SolutionKey const& key : partial_solution) {
    has_cell.at(key.puzzle_row_ - 1).at(key.puzzle_column_ - 1) = true;
  }
  for (auto const& row_ptr : A) {
    SolutionKey const& key = row_ptr.first;
    has_cell.at(key.puzzle_row_ - 1).at(key.puzzle_column_ - 1) = true;
  }
  for (size_t puzzle_row = 1; puzzle_row <= puzzle_size_; ++puzzle_row) {
    for (size_t puzzle_column = 1; puzzle_column <= puzzle_size_; ++puzzle_column) {
      bool c = has_cell.at(puzzle_row - 1).at(puzzle_column - 1);
      if (! c) {
        spdlog::error("cell missing {} {}", puzzle_row, puzzle_column);
        throw std::invalid_argument("puzzle is missing cell"); // TODO change the class
      }
    }
  }
}

void AlgorithmX::cover(AlgorithmX::Matrix &A,
  const AlgorithmX::SolutionKey &r,
  std::set<AlgorithmX::SolutionKey> &partial_solution) const
{
  spdlog::critical("working with {} {} {}", r.puzzle_row_, r.puzzle_column_, r.digit_);

  partial_solution.insert(r);
  //  For each j such that A[r, j] = 1,
  Row const constraints_to_delete = A.at(r);
  //    std::vector<ConstraintKey> constraints_to_delete;
  //    constraints_to_delete.reserve(constraints.size());
  //    for (Row::value_type const& constraint_pair : constraints) {
  //      if (constraint_pair.second) {
  //        constraints_to_delete.push_back(constraint_pair.first);
  //      }
  //    }
  // for each i such that A[i, j] = 1,
  //     delete row i from matrix A.
  delete_row(A, constraints_to_delete);
  print_solution(partial_solution);
  print_matrix(A);
  check_is_valid(partial_solution, A);

  //      delete column j from matrix A;
  // TODO delete_column(A, constraints_to_delete);
}

static bool intersects(std::set<AlgorithmX::ConstraintKey> const &rhs, std::set<AlgorithmX::ConstraintKey> const &lhs)
{
  auto rhs_iter = std::begin(rhs);
  auto lhs_iter = std::begin(lhs);
  while ((rhs_iter != std::end(rhs)) && (lhs_iter != std::end(lhs))) {
    AlgorithmX::ConstraintKey const &p_rhs = *rhs_iter;
    AlgorithmX::ConstraintKey const &p_lhs = *lhs_iter;
    if (p_rhs < p_lhs) {
      ++rhs_iter;
    } else if (p_lhs < p_rhs) {
      ++lhs_iter;
    } else {
      return true;
    }
  }
  return false;
}

//      delete column j from matrix A;
// NOLINTNEXTLINE(passedByValue,performance-unnecessary-value-param)
void AlgorithmX::delete_column(AlgorithmX::Matrix &A, std::set<AlgorithmX::ConstraintKey> const& constraints) // NOLINT
{
  std::begin(A);
  // TODO for (Matrix::value_type &row_pair : A) { row_pair.second.erase(std::begin(constraints), std::end(constraints)); }
}

// for each i such that A[i, j] = 1,
//     delete row i from matrix A.
void AlgorithmX::delete_row(AlgorithmX::Matrix &A, std::set<ConstraintKey> const &constraints)
{
  auto i = A.begin();
  while (i != A.end()) {
    if (intersects(constraints, i->second)) {
      i = A.erase(i);
    } else {
      ++i;
    }
  }
}


}// namespace dlx
