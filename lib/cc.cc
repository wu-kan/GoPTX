#include "GoPTX/GoPTX.hh"
#include <algorithm>
#include <cassert>
#include <deque>
#include <queue>
#include <string>
#include <unordered_map>
#include <unordered_set>
#include <vector>

bool GoPTX::Terminal::Equal::operator()(const GoPTX::Terminal &lhs,
                                        const GoPTX::Terminal &rhs) const {
  return lhs.type == rhs.type && lhs.text == rhs.text;
}

std::string GoPTX::Terminal::to_string() const {
  return leading_space ? " " + text : text;
}

std::string GoPTX::VariableDeclarator::to_string() const {
  std::string ret = id_or_opcode.to_string();
#ifdef GOPTX_VariableDeclarator_FOR
#error
#else
#define GOPTX_VariableDeclarator_FOR(W)                                        \
  do {                                                                         \
    for (const auto &terminal : W) {                                           \
      ret += terminal.to_string();                                             \
    }                                                                          \
  } while (0)
  GOPTX_VariableDeclarator_FOR(parameterized_register_spec);
  GOPTX_VariableDeclarator_FOR(array_spec);
  GOPTX_VariableDeclarator_FOR(equal_initializer);
#undef GOPTX_VariableDeclarator_FOR
#endif
  return ret;
}

std::string GoPTX::Statement::to_string() const {
  std::string ret;
#ifdef GOPTX_STATEMENT_FOR
#error
#else
#define GOPTX_STATEMENT_FOR(W)                                                 \
  do {                                                                         \
    for (const auto &terminal : W) {                                           \
      ret += terminal.to_string();                                             \
    }                                                                          \
  } while (0)
  GOPTX_STATEMENT_FOR(label_decl);
  if (!branch_targets.empty()) {
    ret += " .branchtargets ";
    for (const auto &branch_target : branch_targets) {
      if (ret.back() != ' ') {
        ret += ",";
      }
      ret += branch_target.to_string();
    }
  }
  GOPTX_STATEMENT_FOR(instruction.predicate);
  GOPTX_STATEMENT_FOR(i_bra);
  GOPTX_STATEMENT_FOR(i_ret);
  GOPTX_STATEMENT_FOR(brefore_variable_declarator);
  if (!variable_declarators.empty()) {
    ret += ' ';
    for (const auto &variable_declarator : variable_declarators) {
      if (ret.back() != ' ') {
        ret += ",";
      }
      ret += variable_declarator.to_string();
    }
  }
  GOPTX_STATEMENT_FOR(instruction.instruction_aux);
#undef GOPTX_STATEMENT_FOR
#endif
  ret += "\n";
  return ret;
}

std::string GoPTX::EntryOrFunc::to_string() const {
  std::string ret;
  ret += k_visible.to_string();
  ret += k_extern.to_string();
  ret += k_entry.to_string();
  ret += k_func.to_string();
  if (!func_ret_list.empty()) {
    ret += '(';
    for (const auto &func_ret : func_ret_list) {
      if (ret.back() != '(')
        ret += ",\n";
      for (const auto &terminal : func_ret)
        ret += terminal.to_string();
    }
    ret += ')';
  }
  ret += name.to_string();
  ret += '(';
  for (const auto &param : param_list) {
    if (ret.back() != '(')
      ret += ",";
    for (const auto &terminal : param)
      ret += terminal.to_string();
  }
  ret += ")";
  for (const auto &directive : performance_tuning_directives)
    for (const auto &terminal : directive) {
      ret += terminal.to_string();
    }
  ret += "{\n";
  for (const auto &statement : statements) {
    ret += statement.to_string();
  }

  ret += "}\n";
  return ret;
}

std::string GoPTX::Prog::to_string() const {
  std::string ret;
  for (const auto &v : {version, target, address_size}) {
    for (const auto &terminal : v)
      ret += terminal.to_string();
    ret += "\n";
  }
  for (const auto &statement : statements)
    ret += statement.to_string();
  for (const auto &entry_or_func : entry_or_func_list)
    ret += entry_or_func.to_string();
  return ret;
}

std::pair<int, int> GoPTX::Terminal::match_label_decl_or_variable_declarator(
    const std::vector<GoPTX::Statement> &statements) const {
  GoPTX::Terminal::Equal terminal_equal;
  std::pair<int, int> ret(-1, -1);
  for (const auto &statement : statements) {
    ++ret.first;
    ret.second = -1;
    if (!statement.label_decl.empty()) {
      if (terminal_equal(*this, statement.label_decl.front())) {
        return ret;
      }
    }
    for (const auto &variable_declarator : statement.variable_declarators) {
      ++ret.second;
      if (type == variable_declarator.id_or_opcode.type) {
        if (!variable_declarator.parameterized_register_spec.empty()) {
          if (text.find(variable_declarator.id_or_opcode.text) == 0) {
            auto the_fxxk_parameterized =
                text.substr(variable_declarator.id_or_opcode.text.size());
            int fxxk = 0;
            for (const auto &c : the_fxxk_parameterized) {
              if (!std::isdigit(c)) {
                fxxk = -1;
                break;
              }
              fxxk = fxxk * 10 + c - '0';
            }
            if (the_fxxk_parameterized.empty())
              fxxk = -1;
            if (0 <= fxxk &&
                fxxk <
                    std::stoi(variable_declarator.parameterized_register_spec[1]
                                  .text))
              return ret;
          }
        } else {
          if (text == variable_declarator.id_or_opcode.text)
            return ret;
        }
      }
    }
  }
  ret.first = -1;
  ret.second = -1;
  return ret;
}

std::pair<int, int> GoPTX::Terminal::match_param(
    const std::vector<std::vector<GoPTX::Terminal>> &param_list) const {
  GoPTX::Terminal::Equal terminal_equal;
  std::pair<int, int> ret(-1, -1);
  if (type != GoPTX::Terminal::get_type_T_WORD())
    return ret;
  for (const auto &param : param_list) {
    ++ret.first;
    ret.second = -1;
    for (const auto &terminal : param) {
      ++ret.second;
      if (terminal_equal(*this, terminal))
        return ret;
    }
  }
  ret.first = -1;
  ret.second = -1;
  return ret;
}

void GoPTX::EntryOrFunc::add_suffix(const std::string &suffix) {
  GoPTX::Terminal::Equal terminal_equal;
  if (suffix.empty())
    return;
  for (auto &statement : statements) {
#ifdef GOPTX_STATEMENT_FOR
#error
#else
#define GOPTX_STATEMENT_FOR(W)                                                 \
  do {                                                                         \
    for (auto &terminal : W) {                                                 \
      auto it = terminal.match_label_decl_or_variable_declarator(statements);  \
      if (it.first >= 0) {                                                     \
        if (it.second >= 0) {                                                  \
          if (!statements[it.first]                                            \
                   .variable_declarators[it.second]                            \
                   .parameterized_register_spec.empty()) {                     \
            terminal.text =                                                    \
                statements[it.first]                                           \
                    .variable_declarators[it.second]                           \
                    .id_or_opcode.text +                                       \
                suffix +                                                       \
                terminal.text.substr(statements[it.first]                      \
                                         .variable_declarators[it.second]      \
                                         .id_or_opcode.text.size());           \
          } else {                                                             \
            terminal.text += suffix;                                           \
          }                                                                    \
        } else {                                                               \
          terminal.text += suffix;                                             \
        }                                                                      \
      } else {                                                                 \
        auto jt = terminal.match_param(param_list);                            \
        if (jt.first >= 0) {                                                   \
          terminal.text += suffix;                                             \
        }                                                                      \
      }                                                                        \
    }                                                                          \
  } while (0)
    GOPTX_STATEMENT_FOR(statement.instruction.predicate);
    GOPTX_STATEMENT_FOR(statement.instruction.instruction_aux);
    GOPTX_STATEMENT_FOR(statement.i_bra);
    GOPTX_STATEMENT_FOR(statement.i_ret);
#undef GOPTX_STATEMENT_FOR
#endif
  }
  for (auto &statement : statements) {
    for (auto &variable_declarator : statement.variable_declarators) {
      variable_declarator.id_or_opcode.text += suffix;
    }
    if (!statement.label_decl.empty()) {
      statement.label_decl.front().text += suffix;
    }
  }
  for (auto &param : param_list) {
    for (auto &terminal : param) {
      if (terminal.type == GoPTX::Terminal::get_type_T_WORD()) {
        terminal.text += suffix;
      }
    }
  }
}

void GoPTX::Prog::add_suffix(const std::string &suffix) {
  GoPTX::Terminal::Equal terminal_equal;
  if (suffix.empty())
    return;
  for (auto &entry_or_func : entry_or_func_list) {
    entry_or_func.add_suffix(suffix);
    for (auto &statement : entry_or_func.statements) {
#ifdef GOPTX_STATEMENT_FOR
#error
#else
#define GOPTX_STATEMENT_FOR(W)                                                 \
  do {                                                                         \
    for (auto &terminal : W) {                                                 \
      auto jt = terminal.match_param(entry_or_func.param_list);                \
      if (jt.first >= 0)                                                       \
        continue;                                                              \
      auto it = terminal.match_label_decl_or_variable_declarator(              \
          entry_or_func.statements);                                           \
      if (it.first >= 0)                                                       \
        continue;                                                              \
      auto kt = terminal.match_label_decl_or_variable_declarator(statements);  \
      if (kt.first >= 0) {                                                     \
        if (kt.second >= 0) {                                                  \
          if (!statements[kt.first]                                            \
                   .variable_declarators[kt.second]                            \
                   .parameterized_register_spec.empty()) {                     \
            terminal.text =                                                    \
                statements[kt.first]                                           \
                    .variable_declarators[kt.second]                           \
                    .id_or_opcode.text +                                       \
                suffix +                                                       \
                terminal.text.substr(statements[kt.first]                      \
                                         .variable_declarators[kt.second]      \
                                         .id_or_opcode.text.size());           \
          } else {                                                             \
            terminal.text += suffix;                                           \
          }                                                                    \
        } else {                                                               \
          terminal.text += suffix;                                             \
        }                                                                      \
      } else {                                                                 \
        for (const auto &entry_or_func : entry_or_func_list) {                 \
          if (terminal_equal(terminal, entry_or_func.name)) {                  \
            terminal.text += suffix;                                           \
          }                                                                    \
        }                                                                      \
      }                                                                        \
    }                                                                          \
  } while (0)
      GOPTX_STATEMENT_FOR(statement.instruction.predicate);
      GOPTX_STATEMENT_FOR(statement.instruction.instruction_aux);
      GOPTX_STATEMENT_FOR(statement.i_bra);
      GOPTX_STATEMENT_FOR(statement.i_ret);
#undef GOPTX_STATEMENT_FOR
#endif
    }
  }
  for (auto &statement : statements) {
#ifdef GOPTX_STATEMENT_FOR
#error
#else
#define GOPTX_STATEMENT_FOR(W)                                                 \
  do {                                                                         \
    for (auto &terminal : W) {                                                 \
      auto it = terminal.match_label_decl_or_variable_declarator(statements);  \
      if (it.first >= 0) {                                                     \
        if (it.second >= 0) {                                                  \
          if (!statements[it.first]                                            \
                   .variable_declarators[it.second]                            \
                   .parameterized_register_spec.empty()) {                     \
            terminal.text =                                                    \
                statements[it.first]                                           \
                    .variable_declarators[it.second]                           \
                    .id_or_opcode.text +                                       \
                suffix +                                                       \
                terminal.text.substr(statements[it.first]                      \
                                         .variable_declarators[it.second]      \
                                         .id_or_opcode.text.size());           \
          } else {                                                             \
            terminal.text += suffix;                                           \
          }                                                                    \
        } else {                                                               \
          terminal.text += suffix;                                             \
        }                                                                      \
      }                                                                        \
    }                                                                          \
  } while (0)
    GOPTX_STATEMENT_FOR(statement.instruction.predicate);
    GOPTX_STATEMENT_FOR(statement.instruction.instruction_aux);
    GOPTX_STATEMENT_FOR(statement.i_ret);
    GOPTX_STATEMENT_FOR(statement.i_bra);
#undef GOPTX_STATEMENT_FOR
#endif
  }
  for (auto &statement : statements) {
    for (auto &variable_declarator : statement.variable_declarators) {
      variable_declarator.id_or_opcode.text += suffix;
    }
    if (!statement.label_decl.empty()) {
      statement.label_decl.front().text += suffix;
    }
  }
  for (auto &entry_or_func : entry_or_func_list) {
    entry_or_func.name.text += suffix;
  }
}

std::vector<std::string> GoPTX::Prog::get_identifiers() const {
  std::vector<std::string> identifiers;
  for (const auto &entry_or_func : entry_or_func_list) {
    identifiers.push_back(entry_or_func.name.text);
  }
  for (const auto &statement : statements) {
    for (auto &variable_declarator : statement.variable_declarators) {
      identifiers.push_back(variable_declarator.id_or_opcode.text);
    }
    if (!statement.label_decl.empty()) {
      identifiers.push_back(statement.label_decl.front().text);
    }
  }
  return identifiers;
}

int GoPTX::Prog::concat(GoPTX::Prog prog_second,
                        const std::string &suffix_first,
                        const std::string &suffix_second) {
  GoPTX::Terminal::Equal terminal_equal;
  if (version.size() != prog_second.version.size() ||
      !std::equal(version.begin(), version.end(), prog_second.version.begin(),
                  terminal_equal))
    return -1;
  if (target.size() != prog_second.target.size() ||
      !std::equal(target.begin(), target.end(), prog_second.target.begin(),
                  terminal_equal))
    return -1;
  if (address_size.size() != prog_second.address_size.size() ||
      !std::equal(address_size.begin(), address_size.end(),
                  prog_second.address_size.begin(), terminal_equal))
    return -1;
  if (suffix_first == suffix_second) {
    std::vector<std::string> identifiers_first = get_identifiers(),
                             identifiers_second = prog_second.get_identifiers();
    std::sort(identifiers_first.begin(), identifiers_first.end());
    std::sort(identifiers_second.begin(), identifiers_second.end());
    for (size_t i = 0, j = 0;
         i < identifiers_first.size() && j < identifiers_second.size();) {
      if (identifiers_first[i] == identifiers_second[j])
        return -1;
      if (identifiers_first[i] < identifiers_second[j])
        ++i;
      else
        ++j;
    }
  }
  add_suffix(suffix_first);
  prog_second.add_suffix(suffix_second);
#ifdef GOPTX_ENTRY_OR_FUNC_FOR
#error
#else
#define GOPTX_ENTRY_OR_FUNC_FOR(W)                                             \
  do {                                                                         \
    W.insert(W.end(), prog_second.W.begin(), prog_second.W.end());             \
  } while (0)
  GOPTX_ENTRY_OR_FUNC_FOR(statements);
  GOPTX_ENTRY_OR_FUNC_FOR(entry_or_func_list);
#undef GOPTX_ENTRY_OR_FUNC_FOR
#endif
  return 0;
}

GoPTX::ControlFlowGraph::ControlFlowGraph(
    const std::vector<GoPTX::Statement> &statements)
    : basic_blocks(statements.size() + 1) {
  std::unordered_map<std::string, size_t> label_table;
  for (size_t i = 0; i < statements.size(); ++i) {
    if (!statements[i].label_decl.empty()) {
      assert(statements[i].label_decl.size() == 2);
      assert(!label_table.count(statements[i].label_decl.front().text));
      label_table.emplace(statements[i].label_decl.front().text, i);
    }
  }
  for (size_t i = 0; i < statements.size(); ++i) {
    if (!statements[i].brefore_variable_declarator.empty() ||
        !statements[i].variable_declarators.empty()) {
      identifier_decls.push_back(statements[i]);
      assert(identifier_decls.back().label_decl.empty());
      identifier_decls.back().i_bra.clear();
      identifier_decls.back().i_ret.clear();
      size_t next_0 = i + 1;
      basic_blocks[i].out.push_back(next_0);
    } else if (!statements[i].i_bra.empty()) {
      if (!statements[i].instruction.predicate.empty()) {
        assert(statements[i].instruction.predicate.size() == 2 ||
               statements[i].instruction.predicate.size() == 3);
        basic_blocks[i].bra_predicate =
            statements[i].instruction.predicate.back();
        assert(label_table.count(statements[i].i_bra.back().text));

        size_t next_0 = i + 1,
               next_1 = label_table[statements[i].i_bra.back().text];
        if (statements[i].instruction.predicate.size() == 3)
          std::swap(next_0, next_1);
        basic_blocks[i].out.push_back(next_0);
        basic_blocks[i].out.push_back(next_1);
      } else {
        assert(label_table.count(statements[i].i_bra.back().text));
        size_t next_1 = label_table[statements[i].i_bra.back().text];
        basic_blocks[i].out.push_back(next_1);
      }
    } else if (!statements[i].i_ret.empty()) {
      if (!statements[i].instruction.predicate.empty()) {
        assert(statements[i].instruction.predicate.size() == 2 ||
               statements[i].instruction.predicate.size() == 3);
        basic_blocks[i].bra_predicate =
            statements[i].instruction.predicate.back();
        size_t next_0 = i + 1, next_1 = statements.size();
        if (statements[i].instruction.predicate.size() == 3)
          std::swap(next_0, next_1);
        basic_blocks[i].out.push_back(next_0);
        basic_blocks[i].out.push_back(next_1);
      } else {
        size_t next_1 = statements.size();
        basic_blocks[i].out.push_back(next_1);
      }
    } else {
      if (!statements[i].instruction.instruction_aux.empty())
        basic_blocks[i].instructions.push_back(statements[i].instruction);
      size_t next_0 = i + 1;
      basic_blocks[i].out.push_back(next_0);
    }
  }
  for (auto &basic_block : basic_blocks) {
    for (auto &instruction : basic_block.instructions) {
      bool find_bar = false;
      for (const auto &terminal : instruction.instruction_aux) {
        if (terminal.type == GoPTX::Terminal::get_type_KI_BAR()) {
          find_bar = true;
          break;
        }
      }
      if (find_bar) {
        for (size_t i = 0; i < instruction.instruction_aux.size(); ++i) {
          auto &terminal = instruction.instruction_aux[i];
          if (terminal.type == GoPTX::Terminal::get_type_KI_BAR()) {
            terminal.type = GoPTX::Terminal::get_type_KI_BARRIER();
            terminal.text = "barrier";
          }
          if (terminal.type == GoPTX::Terminal::get_type_K_SYNC()) {
            GoPTX::Terminal terminal_aligned;
            terminal_aligned.type = GoPTX::Terminal::get_type_K_ALIGNED();
            terminal_aligned.text = ".aligned";
            terminal_aligned.leading_space = false;
            instruction.instruction_aux.insert(
                instruction.instruction_aux.begin() + i + 1, terminal_aligned);
            break;
          }
        }
      }
    }
  }
  fuse_basic_block();
  remove_dead_basic_block();
}

std::string GoPTX::ControlFlowGraph::get_brx_index_name(size_t index) const {
  return "%" + std::to_string(index) + "__brx_index_GoPTX";
}

std::vector<GoPTX::Statement>
GoPTX::ControlFlowGraph::to_statements(const std::string &label_suffix,
                                       const std::string &label_prefix) const {
#ifdef DEFINE_TERMINAL
#error
#else
#define DEFINE_TERMINAL(T, TYPE, TEXT, LEADING_SPACE)                          \
  GoPTX::Terminal T;                                                           \
  do {                                                                         \
    T.type = GoPTX::Terminal::get_type_##TYPE();                               \
    T.text = TEXT;                                                             \
    T.leading_space = LEADING_SPACE;                                           \
  } while (0);
  std::vector<GoPTX::Statement> ret = identifier_decls;
  for (size_t i = 0; i < basic_blocks.size(); ++i) {
    do {
      GoPTX::Statement label_of_block;
      DEFINE_TERMINAL(label, T_WORD,
                      label_prefix + std::to_string(i) + label_suffix, true)
      DEFINE_TERMINAL(t_colon, T_COLON, ":", false)
      label_of_block.label_decl.assign({label, t_colon});
      ret.push_back(label_of_block);
    } while (0);
    for (const auto &instruction : basic_blocks[i].instructions) {
      GoPTX::Statement statement;
      statement.instruction = instruction;
      ret.push_back(statement);
    }
    DEFINE_TERMINAL(t_semicolon, T_SEMICOLON, ";", false)
    if (basic_blocks[i].out.size() == 0) {
      if (basic_blocks[i].brx_out.empty()) {
        DEFINE_TERMINAL(ki_ret, KI_RET, "ret", true)
        GoPTX::Statement statement;
        statement.i_ret.assign({ki_ret});
        statement.instruction.instruction_aux.assign({t_semicolon});
        ret.push_back(statement);
      } else {
        DEFINE_TERMINAL(label_tlist, T_WORD,
                        label_prefix + std::to_string(i) + "__tilist_GoPTX__" +
                            label_suffix,
                        true)
        do {
          DEFINE_TERMINAL(t_colon, T_COLON, ":", false)
          GoPTX::Statement statement;
          statement.label_decl.assign({label_tlist, t_colon});
          for (const auto &brx_idxs : basic_blocks[i].brx_out) {
            DEFINE_TERMINAL(
                label, T_WORD,
                label_prefix + std::to_string(brx_idxs) + label_suffix, true)
            statement.branch_targets.push_back(label);
          }
          statement.instruction.instruction_aux.assign({t_semicolon});
          ret.push_back(statement);
        } while (0);
        do {
          DEFINE_TERMINAL(ki_brx, KI_BRX, "brx", true)
          DEFINE_TERMINAL(k_idx, K_IDX, ".idx", false)
          DEFINE_TERMINAL(t_comma, T_COMMA, ",", false)
          GoPTX::Statement statement;
          statement.instruction.instruction_aux.assign(
              {ki_brx, k_idx, basic_blocks[i].brx_index, t_comma, label_tlist,
               t_semicolon});
          ret.push_back(statement);
        } while (0);
      }
    } else {
      auto front = basic_blocks[i].out.front(),
           back = basic_blocks[i].out.back();
      if (back != front) {
        GoPTX::Statement statement;
        DEFINE_TERMINAL(t_at, T_AT, "@", true)
        statement.instruction.predicate.push_back(t_at);
        if (back == i + 1) {
          std::swap(back, front);
          DEFINE_TERMINAL(t_not, T_NOT, "!", false)
          statement.instruction.predicate.push_back(t_not);
        }
        DEFINE_TERMINAL(ki_bra, KI_BRA, "bra", true)
        DEFINE_TERMINAL(label, T_WORD,
                        label_prefix + std::to_string(back) + label_suffix,
                        true)
        statement.instruction.predicate.push_back(
            basic_blocks[i].bra_predicate);
        statement.i_bra.assign({ki_bra, label});
        statement.instruction.instruction_aux.assign({t_semicolon});
        ret.push_back(statement);
      }
      if (front != i + 1) {
        DEFINE_TERMINAL(ki_bra, KI_BRA, "bra", true)
        DEFINE_TERMINAL(label, T_WORD,
                        label_prefix + std::to_string(front) + label_suffix,
                        true)
        GoPTX::Statement statement;
        statement.i_bra.assign({ki_bra, label});
        statement.instruction.instruction_aux.assign({t_semicolon});
        ret.push_back(statement);
      }
    }
  }
  return ret;
#undef DEFINE_TERMINAL
#endif
}

void GoPTX::ControlFlowGraph::fuse_basic_block() {
  for (size_t i = 0; i < basic_blocks.size(); ++i) {
    if (i != 0 && basic_blocks[i].brx_out.empty() &&
        basic_blocks[i].out.size() == 1 &&
        basic_blocks[i].instructions.empty()) {
      size_t j = basic_blocks[i].out.back();
      for (auto &basic_block : basic_blocks) {
        for (auto &k : basic_block.out)
          if (k == i)
            k = j;
        for (auto &k : basic_block.brx_out)
          if (k == i)
            k = j;
      }
      basic_blocks[i].out.clear();
    }
  }
  const auto has_aligned =
      get_has_keyword(GoPTX::Terminal::get_type_K_ALIGNED());
  for (size_t i = 0; i < basic_blocks.size(); ++i) {
    while (basic_blocks[i].brx_out.empty() && basic_blocks[i].out.size() == 1) {
      size_t j = basic_blocks[i].out.back();
      if (i != j && !has_aligned[j]) {
        basic_blocks[i].instructions.insert(
            basic_blocks[i].instructions.end(),
            basic_blocks[j].instructions.begin(),
            basic_blocks[j].instructions.end());
        basic_blocks[i].bra_predicate = basic_blocks[j].bra_predicate;
        basic_blocks[i].brx_out = basic_blocks[j].brx_out;
        basic_blocks[i].out = basic_blocks[j].out;
      } else
        break;
    }
  }
}

void GoPTX::ControlFlowGraph::remove_dead_basic_block() {
  std::vector<size_t> table, stak(1, 0),
      stak_index(basic_blocks.size(), basic_blocks.size()),
      rtable(basic_blocks.size(), basic_blocks.size());
  stak_index[0] = 0;
  size_t cnt = 0;
  while (!stak.empty()) {
    auto u = stak.back();
    stak.pop_back();
    table.push_back(u);
    for (const auto &v : basic_blocks[u].out) {
      if (stak_index[v] == basic_blocks.size()) {
        stak_index[v] = stak.size();
        stak.push_back(v);
      }
    }
    for (const auto &v : basic_blocks[u].brx_out) {
      if (stak_index[v] == basic_blocks.size()) {
        stak_index[v] = stak.size();
        stak.push_back(v);
      }
    }
    if (!basic_blocks[u].out.empty()) {
      auto v = basic_blocks[u].out.front(), vv = basic_blocks[u].out.back();
      if ((stak_index[v] > stak.size() || stak[stak_index[v]] != v) &&
          stak_index[vv] < stak.size() && stak[stak_index[vv]] == vv)
        std::swap(v, vv);
      if (stak_index[v] < stak.size() && stak[stak_index[v]] == v) {
        auto w = stak.back();
        if (w != v) {
          std::swap(stak_index[v], stak_index[w]);
          std::swap(stak[stak_index[v]], stak[stak_index[w]]);
        }
      }
    }
  }
  for (size_t i = 0; i < table.size(); ++i)
    rtable[table[i]] = i;
  std::vector<GoPTX::ControlFlowGraph::BasicBlock> new_basic_blocks;
  for (size_t i = 0; i < table.size(); ++i) {
    new_basic_blocks.push_back(basic_blocks[table[i]]);
    for (auto &v : new_basic_blocks.back().out)
      v = rtable[v];
    for (auto &v : new_basic_blocks.back().brx_out)
      v = rtable[v];
  }
  std::swap(basic_blocks, new_basic_blocks);
}

std::string GoPTX::ControlFlowGraph::to_mermaid() const {
  std::string ret = "flowchart TB\n";
  const auto has_sync = get_has_keyword(GoPTX::Terminal::get_type_K_SYNC()),
             before_sync = get_before_sync();
  for (size_t i = 0; i < basic_blocks.size(); ++i) {
    if (has_sync[i]) {
      ret += std::to_string(i) + "{{" + std::to_string(i) + "}}\n";
    } else if (before_sync[i]) {
      ret += std::to_string(i) + ">" + std::to_string(i) + "]\n";
    }
    if (basic_blocks[i].out.size() == 2) {
      auto back = basic_blocks[i].out.back(),
           front = basic_blocks[i].out.front();
      ret += std::to_string(i) + "--" + basic_blocks[i].bra_predicate.text +
             "-->" + std::to_string(back) + "\n";
      ret += std::to_string(i) + "--!" + basic_blocks[i].bra_predicate.text +
             "-->" + std::to_string(front) + "\n";
    } else if (basic_blocks[i].out.size() == 1) {
      auto front = basic_blocks[i].out.front();
      ret += std::to_string(i) + "--->" + std::to_string(front) + "\n";
    }
  }
  return ret;
}

void GoPTX::ControlFlowGraph::replace_aligned() {
  for (auto &basic_block : basic_blocks)
    for (auto &instruction : basic_block.instructions) {
      if (instruction.instruction_aux.front().type ==
          GoPTX::Terminal::get_type_KI_BARRIER()) {
        for (size_t i = 0; i < instruction.instruction_aux.size(); ++i) {
          if (instruction.instruction_aux[i].type ==
              GoPTX::Terminal::get_type_K_ALIGNED()) {
            instruction.instruction_aux.erase(
                instruction.instruction_aux.begin() + i,
                instruction.instruction_aux.begin() + i + 1);
            break;
          }
        }
      }
    }
}

void GoPTX::ControlFlowGraph::split_aligned_block() {
  for (size_t i = 0; i < basic_blocks.size(); ++i) {
    size_t sum_lat = 0;
    bool is_aligned_0, is_aligned_j;
    for (size_t j = 0; j < basic_blocks[i].instructions.size(); ++j) {
      is_aligned_j = false;
      for (const auto &terminal :
           basic_blocks[i].instructions[j].instruction_aux) {
        if (terminal.type == GoPTX::Terminal::get_type_K_ALIGNED()) {
          is_aligned_j = true;
          break;
        }
      }
      if (j == 0)
        is_aligned_0 = is_aligned_j;
      if (is_aligned_0 != is_aligned_j) {
        size_t max_basic_block_size = j;
        BasicBlock new_bb;
        new_bb.instructions.assign(basic_blocks[i].instructions.begin() +
                                       max_basic_block_size,
                                   basic_blocks[i].instructions.end());
        new_bb.out.assign(1, basic_blocks.size());
        std::swap(basic_blocks[i].out, new_bb.out);
        std::swap(basic_blocks[i].bra_predicate, new_bb.bra_predicate);
        basic_blocks[i].instructions.erase(
            basic_blocks[i].instructions.begin() + max_basic_block_size,
            basic_blocks[i].instructions.end());
        basic_blocks.push_back(new_bb);
      }
    }
  }
}

void GoPTX::ControlFlowGraph::split_basic_block_by_latency() {
  size_t latency = 0;
  for (size_t i = 0; i < basic_blocks.size(); ++i) {
    for (size_t j = 0; j < basic_blocks[i].instructions.size(); ++j) {
      latency +=
          GoPTX::get_instruction_latency(basic_blocks[i].instructions[j]);
    }
  }
  latency /= std::max<size_t>(1, basic_blocks.size());
  for (size_t i = 0; i < basic_blocks.size(); ++i) {
    size_t sum_lat = 0;
    for (size_t j = 0; j < basic_blocks[i].instructions.size(); ++j) {
      sum_lat +=
          GoPTX::get_instruction_latency(basic_blocks[i].instructions[j]);
      if (j && sum_lat > latency) {
        size_t max_basic_block_size = j;
        BasicBlock new_bb;
        new_bb.instructions.assign(basic_blocks[i].instructions.begin() +
                                       max_basic_block_size,
                                   basic_blocks[i].instructions.end());
        new_bb.out.assign(1, basic_blocks.size());
        std::swap(basic_blocks[i].out, new_bb.out);
        std::swap(basic_blocks[i].bra_predicate, new_bb.bra_predicate);
        basic_blocks[i].instructions.erase(
            basic_blocks[i].instructions.begin() + max_basic_block_size,
            basic_blocks[i].instructions.end());
        basic_blocks.push_back(new_bb);
      }
    }
  }
}

std::vector<int8_t>
GoPTX::ControlFlowGraph::get_has_keyword(size_t keyword) const {
  std::vector<int8_t> has_sync(basic_blocks.size(), 0);
  for (size_t i = 0; i < basic_blocks.size(); ++i) {
    for (const auto &instruction : basic_blocks[i].instructions) {
      for (const auto &terminal : instruction.instruction_aux) {
        if (terminal.type == keyword) {
          has_sync[i] = 1;
          break;
        }
      }
      if (has_sync[i])
        break;
    }
  }
  return has_sync;
}

std::vector<int8_t> GoPTX::ControlFlowGraph::get_before_sync() const {
  std::vector<std::vector<size_t>> in(basic_blocks.size());
  std::vector<int8_t> has_sync =
                          get_has_keyword(GoPTX::Terminal::get_type_K_SYNC()),
                      before_sync(basic_blocks.size(), 0);
  std::vector<size_t> stak;
  for (size_t i = 0; i < basic_blocks.size(); ++i) {
    for (auto j : basic_blocks[i].out)
      in[j].push_back(i);
    if (has_sync[i]) {
      stak.push_back(i);
      before_sync[i] = 1;
    }
  }
  while (!stak.empty()) {
    auto u = stak.back();
    stak.pop_back();
    for (auto v : in[u]) {
      if (!before_sync[v]) {
        before_sync[v] = 1;
        stak.push_back(v);
      }
    }
  }
  return before_sync;
}

GoPTX::ControlFlowGraph
GoPTX::StrategyVFuse::operator()(const GoPTX::ControlFlowGraph &cfg_first,
                                 const GoPTX::ControlFlowGraph &cfg_second,
                                 const GoPTX_config &config) const {
  GoPTX::ControlFlowGraph ret = cfg_first;
  ret.identifier_decls.insert(ret.identifier_decls.end(),
                              cfg_second.identifier_decls.begin(),
                              cfg_second.identifier_decls.end());
  ret.basic_blocks.insert(ret.basic_blocks.end(),
                          cfg_second.basic_blocks.begin(),
                          cfg_second.basic_blocks.end());
  for (auto j = cfg_first.basic_blocks.size(); j < ret.basic_blocks.size();
       ++j) {
    for (auto &k : ret.basic_blocks[j].out) {
      k += cfg_first.basic_blocks.size();
    }
  }
  for (auto j = cfg_first.basic_blocks.size(), i = j - j; i < j; ++i) {
    if (ret.basic_blocks[i].out.empty()) {
      ret.basic_blocks[i].out.push_back(j);
    }
  }
  ret.fuse_basic_block();
  ret.remove_dead_basic_block();
  return ret;
}

GoPTX::ControlFlowGraph
GoPTX::StrategyHFuse::operator()(const GoPTX::ControlFlowGraph &cfg_first,
                                 const GoPTX::ControlFlowGraph &cfg_second,
                                 const GoPTX_config &config) const {
  std::vector<GoPTX::Statement> statements;
#ifdef DEFINE_TERMINAL
#error
#else
#define DEFINE_TERMINAL(T, TYPE, TEXT, LEADING_SPACE)                          \
  GoPTX::Terminal T;                                                           \
  do {                                                                         \
    T.type = GoPTX::Terminal::get_type_##TYPE();                               \
    T.text = TEXT;                                                             \
    T.leading_space = LEADING_SPACE;                                           \
  } while (0);
  DEFINE_TERMINAL(reg, K_REG, ".reg", false)
  DEFINE_TERMINAL(pred, K_PRED, ".pred", false)
  DEFINE_TERMINAL(b32, K_B32, ".b32", false)
  DEFINE_TERMINAL(p, T_WORD, "p__hfuse_GoPTX", true)
  DEFINE_TERMINAL(r0, T_WORD, "r0__hfuse_GoPTX", true)
  DEFINE_TERMINAL(r1, T_WORD, "r1__hfuse_GoPTX", true)
  DEFINE_TERMINAL(r2, T_WORD, "r2__hfuse_GoPTX", true)
  DEFINE_TERMINAL(r3, T_WORD, "r3__hfuse_GoPTX", true)
  DEFINE_TERMINAL(mov, KI_MOV, "mov", false)
  DEFINE_TERMINAL(u32, K_U32, ".u32", false)
  DEFINE_TERMINAL(ntid, T_WORD, "%ntid", true)
  DEFINE_TERMINAL(x, K_X, ".x", false)
  DEFINE_TERMINAL(y, K_Y, ".y", false)
  DEFINE_TERMINAL(z, K_Z, ".z", false)
  DEFINE_TERMINAL(mul, KI_MUL, "mul", false)
  DEFINE_TERMINAL(lo, K_LO, ".lo", false)
  DEFINE_TERMINAL(tid, T_WORD, "%tid", true)
  DEFINE_TERMINAL(setp, KI_SETP, "setp", false)
  DEFINE_TERMINAL(ne, K_NE, ".ne", false)
  DEFINE_TERMINAL(zero, T_DEC_LITERAL, "0", true)
  DEFINE_TERMINAL(one, T_DEC_LITERAL, "1", true)
  DEFINE_TERMINAL(barrier, KI_BARRIER, "barrier", false)
  DEFINE_TERMINAL(aligned, K_ALIGNED, ".aligned", false)
  DEFINE_TERMINAL(sync, K_SYNC, ".sync", false)
  DEFINE_TERMINAL(comma, T_COMMA, ",", false)
  DEFINE_TERMINAL(semicolon, T_SEMICOLON, ";", false)
#undef DEFINE_TERMINAL
#endif
  do {
    GoPTX::Statement statement;
    statement.instruction.instruction_aux.assign({semicolon});
    GoPTX::VariableDeclarator vd;
    vd.id_or_opcode = p;
    statement.brefore_variable_declarator.push_back(reg);
    statement.brefore_variable_declarator.push_back(pred);
    statement.variable_declarators.push_back(vd);
    statements.push_back(statement);
    statement.brefore_variable_declarator.back() = b32;
    statement.variable_declarators.clear();
    for (const auto &r : {r0, r1, r2, r3}) {
      vd.id_or_opcode = r;
      statement.variable_declarators.push_back(vd);
    }
    statements.push_back(statement);
  } while (0);
  do {
    GoPTX::Statement statement;
    statement.instruction.instruction_aux.assign(
        {mov, u32, r2, comma, ntid, y, semicolon});
    statements.push_back(statement);
    statement.instruction.instruction_aux.assign(
        {mov, u32, r3, comma, ntid, x, semicolon});
    statements.push_back(statement);
    statement.instruction.instruction_aux.assign(
        {mul, lo, u32, r0, comma, r3, comma, r2, semicolon});
    statements.push_back(statement);
    statement.instruction.instruction_aux.assign(
        {mov, u32, r1, comma, tid, z, semicolon});
    statements.push_back(statement);
    statement.instruction.instruction_aux.assign(
        {setp, ne, u32, p, comma, r1, comma, zero, semicolon});
    statements.push_back(statement);
  } while (0);
  GoPTX::ControlFlowGraph ret(statements);
  ret.basic_blocks.front().bra_predicate = p;
  for (const auto &cfg : {cfg_first, cfg_second}) {
    std::vector<GoPTX::Terminal> bar_sync{barrier, sync, aligned,  zero,
                                          comma,   r0,   semicolon},
        barrier_sync{barrier, sync, zero, comma, r0, semicolon};
    if (!ret.basic_blocks.front().out.empty()) {
      bar_sync[3] = one;
      barrier_sync[2] = one;
    }
    ret.identifier_decls.insert(ret.identifier_decls.end(),
                                cfg.identifier_decls.begin(),
                                cfg.identifier_decls.end());
    ret.basic_blocks.front().out.push_back(ret.basic_blocks.size());
    ret.basic_blocks.insert(ret.basic_blocks.end(), cfg.basic_blocks.begin(),
                            cfg.basic_blocks.end());
    for (auto i = ret.basic_blocks.front().out.back();
         i < ret.basic_blocks.size(); ++i) {
      for (auto &j : ret.basic_blocks[i].out) {
        j += ret.basic_blocks.front().out.back();
      }
      for (auto &instruction : ret.basic_blocks[i].instructions) {
        if (instruction.instruction_aux.front().type ==
            GoPTX::Terminal::get_type_KI_BARRIER()) {
          bool has_aligned = false;
          for (const auto &terminal : instruction.instruction_aux) {
            if (terminal.type == GoPTX::Terminal::get_type_K_ALIGNED()) {
              has_aligned = true;
              break;
            }
          }
          if (!has_aligned) {
            instruction.instruction_aux = barrier_sync;
            continue;
          }
        }
        if (instruction.instruction_aux.front().type ==
                GoPTX::Terminal::get_type_KI_BAR() ||
            instruction.instruction_aux.front().type ==
                GoPTX::Terminal::get_type_KI_BARRIER()) {
          instruction.instruction_aux = bar_sync;
        }
      }
    }
  }
  ret.fuse_basic_block();
  ret.remove_dead_basic_block();
  return ret;
}

template <typename BlockStrategy>
GoPTX::ControlFlowGraph GoPTX::StrategyMerge<BlockStrategy>::operator()(
    const GoPTX::ControlFlowGraph &cfg_first,
    const GoPTX::ControlFlowGraph &cfg_second,
    const GoPTX_config &config) const {

  auto no_avoid_deadlock = config.no_avoid_deadlock;
  auto no_avoid_aligned = config.no_avoid_aligned;

  assert(no_avoid_aligned == true);
  // Aligned instructions can be handled by brx.idx instructions, and the
  // complete strategy codes (already in the comments below) are enabled in our
  // following work XFuse
#ifdef DEFINE_TERMINAL
#error
#else
#define DEFINE_TERMINAL(T, TYPE, TEXT, LEADING_SPACE)                          \
  GoPTX::Terminal T;                                                           \
  do {                                                                         \
    T.type = GoPTX::Terminal::get_type_##TYPE();                               \
    T.text = TEXT;                                                             \
    T.leading_space = LEADING_SPACE;                                           \
  } while (0);

  GoPTX::ControlFlowGraph ret = cfg_first;
  ret.identifier_decls.insert(ret.identifier_decls.end(),
                              cfg_second.identifier_decls.begin(),
                              cfg_second.identifier_decls.end());
  ret.basic_blocks.clear();
  if (cfg_first.basic_blocks.empty() || cfg_second.basic_blocks.empty()) {
    ret.basic_blocks.insert(ret.basic_blocks.end(),
                            cfg_first.basic_blocks.begin(),
                            cfg_first.basic_blocks.end());
    ret.basic_blocks.insert(ret.basic_blocks.end(),
                            cfg_second.basic_blocks.begin(),
                            cfg_second.basic_blocks.end());
    return ret;
  }
  std::vector<int8_t> before_sync_first = cfg_first.get_before_sync(),
                      has_sync_second = cfg_second.get_has_keyword(
                          GoPTX::Terminal::get_type_K_SYNC());
#define GOPTX_NO_USE_BRX
#ifndef GOPTX_NO_USE_BRX
  DEFINE_TERMINAL(t_semicolon, T_SEMICOLON, ";", false)
  std::vector<int8_t> has_aligned_first = cfg_first.get_has_keyword(
                          GoPTX::Terminal::get_type_K_ALIGNED()),
                      has_aligned_second = cfg_second.get_has_keyword(
                          GoPTX::Terminal::get_type_K_ALIGNED());
  ret.basic_blocks.resize((cfg_first.basic_blocks.size() + 1) *
                              (cfg_second.basic_blocks.size() + 1) +
                          cfg_first.basic_blocks.size() +
                          cfg_second.basic_blocks.size());
  size_t brx_offset_first = (cfg_first.basic_blocks.size() + 1) *
                            (cfg_second.basic_blocks.size() + 1),
         brx_offset_second = brx_offset_first + cfg_first.basic_blocks.size();
#else
  ret.basic_blocks.resize((cfg_first.basic_blocks.size() + 1) *
                          (cfg_second.basic_blocks.size() + 1));
#endif

  BlockStrategy block_strategy;
  std::vector<size_t> stak(1, 0),
      table(ret.basic_blocks.size(), ret.basic_blocks.size());
  table[0] = 0;
  while (!stak.empty()) {
    auto u = stak.back();
    stak.pop_back();
    auto u_first = u / (cfg_second.basic_blocks.size() + 1),
         u_second = u - u_first * (cfg_second.basic_blocks.size() + 1);

    if (u_first < cfg_first.basic_blocks.size() &&
        u_second < cfg_second.basic_blocks.size() &&
        ((!no_avoid_deadlock && before_sync_first[u_first] &&
          has_sync_second[u_second])
#ifndef GOPTX_NO_USE_BRX
         || (has_aligned_first[u_first] && has_aligned_second[u_second])
#endif
             )) {
#ifndef GOPTX_NO_USE_BRX
      if (has_aligned_first[u_first] && has_aligned_second[u_second]) {
        size_t that_index = brx_offset_first + u_first;
        DEFINE_TERMINAL(ki_mov, KI_MOV, "mov", true)
        DEFINE_TERMINAL(k_u32, K_U32, ".u32", false)
        DEFINE_TERMINAL(brx_index_name, T_WORD,
                        ret.get_brx_index_name(that_index), true)
        DEFINE_TERMINAL(t_comma, T_COMMA, ",", false)
        DEFINE_TERMINAL(
            brx_idx, T_DEC_LITERAL,
            std::to_string(ret.basic_blocks[that_index].brx_out.size()), true)
        GoPTX::Instruction instruction;
        instruction.instruction_aux.assign(
            {ki_mov, k_u32, brx_index_name, t_comma, brx_idx, t_semicolon});
        ret.basic_blocks[u].instructions.push_back(instruction);
        ret.basic_blocks[u].out.push_back(that_index);
        u = ret.basic_blocks.size();
        ret.basic_blocks[that_index].brx_out.push_back(u);
        GoPTX::ControlFlowGraph::BasicBlock basic_block;
        ret.basic_blocks.push_back(basic_block);
      } else
#endif
      {
        ret.basic_blocks[u].instructions =
            cfg_first.basic_blocks[u_first].instructions;
      }

      ret.basic_blocks[u].bra_predicate =
          cfg_first.basic_blocks[u_first].bra_predicate;

      if (cfg_first.basic_blocks[u_first].out.empty()) {
        auto v_first = cfg_first.basic_blocks.size(), v_second = u_second,
             v = v_first * (cfg_second.basic_blocks.size() + 1) + v_second;
        if (table[v] == table.size()) {
          table[v] = v;
          stak.push_back(v);
        }
        ret.basic_blocks[u].out.push_back(v);
      }
      for (auto v_first : cfg_first.basic_blocks[u_first].out) {
        auto v_second = u_second,
             v = v_first * (cfg_second.basic_blocks.size() + 1) + v_second;
        if (table[v] == table.size()) {
          table[v] = v;
          stak.push_back(v);
        }
        ret.basic_blocks[u].out.push_back(v);
      }
      continue;
    }

    // 接下来处理一方为空节点的情况
#ifndef GOPTX_NO_USE_BRX
    if (u_first < cfg_first.basic_blocks.size() && has_aligned_first[u_first]) {
      if (u_second < cfg_second.basic_blocks.size()) {
        ret.basic_blocks[u].instructions =
            cfg_second.basic_blocks[u_second].instructions;
      }
      size_t that_index = brx_offset_first + u_first;
      DEFINE_TERMINAL(ki_mov, KI_MOV, "mov", true)
      DEFINE_TERMINAL(k_u32, K_U32, ".u32", false)
      DEFINE_TERMINAL(brx_index_name, T_WORD,
                      ret.get_brx_index_name(that_index), true)
      DEFINE_TERMINAL(t_comma, T_COMMA, ",", false)
      DEFINE_TERMINAL(
          brx_idx, T_DEC_LITERAL,
          std::to_string(ret.basic_blocks[that_index].brx_out.size()), true)
      GoPTX::Instruction instruction;
      instruction.instruction_aux.assign(
          {ki_mov, k_u32, brx_index_name, t_comma, brx_idx, t_semicolon});
      ret.basic_blocks[u].instructions.push_back(instruction);
      ret.basic_blocks[u].out.push_back(that_index);
      u = ret.basic_blocks.size();
      ret.basic_blocks[that_index].brx_out.push_back(u);
      GoPTX::ControlFlowGraph::BasicBlock basic_block;
      ret.basic_blocks.push_back(basic_block);
    } else if (u_second < cfg_second.basic_blocks.size() &&
               has_aligned_second[u_second]) {
      if (u_first < cfg_first.basic_blocks.size()) {
        ret.basic_blocks[u].instructions =
            cfg_first.basic_blocks[u_first].instructions;
      }
      size_t that_index = brx_offset_second + u_second;
      DEFINE_TERMINAL(ki_mov, KI_MOV, "mov", true)
      DEFINE_TERMINAL(k_u32, K_U32, ".u32", false)
      DEFINE_TERMINAL(brx_index_name, T_WORD,
                      ret.get_brx_index_name(that_index), true)
      DEFINE_TERMINAL(t_comma, T_COMMA, ",", false)
      DEFINE_TERMINAL(
          brx_idx, T_DEC_LITERAL,
          std::to_string(ret.basic_blocks[that_index].brx_out.size()), true)
      GoPTX::Instruction instruction;
      instruction.instruction_aux.assign(
          {ki_mov, k_u32, brx_index_name, t_comma, brx_idx, t_semicolon});
      ret.basic_blocks[u].instructions.push_back(instruction);
      ret.basic_blocks[u].out.push_back(that_index);
      u = ret.basic_blocks.size();
      ret.basic_blocks[that_index].brx_out.push_back(u);
      GoPTX::ControlFlowGraph::BasicBlock basic_block;
      ret.basic_blocks.push_back(basic_block);
    } else
#endif
        if (u_first < cfg_first.basic_blocks.size() &&
            u_second >= cfg_second.basic_blocks.size()) {
      ret.basic_blocks[u].instructions =
          cfg_first.basic_blocks[u_first].instructions;

    } else if (u_second < cfg_second.basic_blocks.size() &&
               (u_first >= cfg_first.basic_blocks.size())) {
      ret.basic_blocks[u].instructions =
          cfg_second.basic_blocks[u_second].instructions;
    } else {
      // 小优化，将有 sync 的一方放在后面
      if (has_sync_second[u_second]) {
        ret.basic_blocks[u].instructions =
            block_strategy(cfg_first.basic_blocks[u_first].instructions,
                           cfg_second.basic_blocks[u_second].instructions);
      } else {
        ret.basic_blocks[u].instructions =
            block_strategy(cfg_second.basic_blocks[u_second].instructions,
                           cfg_first.basic_blocks[u_first].instructions);
      }
    }

    // 开始处理后继，先处理两者都没有后继的情况
    if (u_first >= cfg_first.basic_blocks.size() ||
        cfg_first.basic_blocks[u_first].out.empty()) {
      if (u_second >= cfg_second.basic_blocks.size() ||
          cfg_second.basic_blocks[u_second].out.empty()) {
        continue;
      }
    }

    // 处理其中一方没有后继的情况
    if (u_first >= cfg_first.basic_blocks.size() ||
        cfg_first.basic_blocks[u_first].out.empty()) {
      ret.basic_blocks[u].bra_predicate =
          cfg_second.basic_blocks[u_second].bra_predicate;
      for (auto v_second : cfg_second.basic_blocks[u_second].out) {
        auto v_first = cfg_first.basic_blocks.size(),
             v = v_first * (cfg_second.basic_blocks.size() + 1) + v_second;
        if (table[v] == table.size()) {
          table[v] = v;
          stak.push_back(v);
        }
        ret.basic_blocks[u].out.push_back(v);
      }
      continue;
    }

    if (u_second >= cfg_second.basic_blocks.size() ||
        cfg_second.basic_blocks[u_second].out.empty()) {
      ret.basic_blocks[u].bra_predicate =
          cfg_first.basic_blocks[u_first].bra_predicate;
      for (auto v_first : cfg_first.basic_blocks[u_first].out) {
        auto v_second = cfg_second.basic_blocks.size(),
             v = v_first * (cfg_second.basic_blocks.size() + 1) + v_second;
        if (table[v] == table.size()) {
          table[v] = v;
          stak.push_back(v);
        }
        ret.basic_blocks[u].out.push_back(v);
      }
      continue;
    }

    // 处理各有两个后继的情况
    if (cfg_first.basic_blocks[u_first].out.size() == 2 &&
        cfg_second.basic_blocks[u_second].out.size() == 2) {
      ret.basic_blocks[u].bra_predicate =
          cfg_first.basic_blocks[u_first].bra_predicate;
      do {
        GoPTX::ControlFlowGraph::BasicBlock basic_block;
        basic_block.bra_predicate =
            cfg_second.basic_blocks[u_second].bra_predicate;
        const auto v_first = cfg_first.basic_blocks[u_first].out.front();
        auto v_second = cfg_second.basic_blocks[u_second].out.front();
        auto v = v_first * (cfg_second.basic_blocks.size() + 1) + v_second;
        basic_block.out.push_back(v);
        v_second = cfg_second.basic_blocks[u_second].out.back();
        v = v_first * (cfg_second.basic_blocks.size() + 1) + v_second;
        basic_block.out.push_back(v);
        ret.basic_blocks[u].out.push_back(ret.basic_blocks.size());
        ret.basic_blocks.push_back(basic_block);
      } while (0);
      do {
        GoPTX::ControlFlowGraph::BasicBlock basic_block;
        basic_block.bra_predicate =
            cfg_second.basic_blocks[u_second].bra_predicate;
        const auto v_first = cfg_first.basic_blocks[u_first].out.back();
        auto v_second = cfg_second.basic_blocks[u_second].out.front();
        auto v = v_first * (cfg_second.basic_blocks.size() + 1) + v_second;
        basic_block.out.push_back(v);
        v_second = cfg_second.basic_blocks[u_second].out.back();
        v = v_first * (cfg_second.basic_blocks.size() + 1) + v_second;
        basic_block.out.push_back(v);
        ret.basic_blocks[u].out.push_back(ret.basic_blocks.size());
        ret.basic_blocks.push_back(basic_block);
      } while (0);
      for (auto v_first : cfg_first.basic_blocks[u_first].out) {
        for (auto v_second : cfg_second.basic_blocks[u_second].out) {
          auto v = v_first * (cfg_second.basic_blocks.size() + 1) + v_second;
          if (table[v] == table.size()) {
            table[v] = v;
            stak.push_back(v);
          }
        }
      }
      continue;
    }

    // 处理只有一方有两个后继的情况
    if (cfg_first.basic_blocks[u_first].out.size() == 2)
      ret.basic_blocks[u].bra_predicate =
          cfg_first.basic_blocks[u_first].bra_predicate;
    else if (cfg_second.basic_blocks[u_second].out.size() == 2)
      ret.basic_blocks[u].bra_predicate =
          cfg_second.basic_blocks[u_second].bra_predicate;
    for (auto v_first : cfg_first.basic_blocks[u_first].out) {
      for (auto v_second : cfg_second.basic_blocks[u_second].out) {
        auto v = v_first * (cfg_second.basic_blocks.size() + 1) + v_second;
        if (table[v] == table.size()) {
          table[v] = v;
          stak.push_back(v);
        }
        ret.basic_blocks[u].out.push_back(v);
      }
    }
  }
#ifndef GOPTX_NO_USE_BRX
  DEFINE_TERMINAL(reg, K_REG, ".reg", false)
  DEFINE_TERMINAL(u32, K_U32, ".u32", false)
  for (size_t i = 0; i < cfg_first.basic_blocks.size(); ++i) {
    if (!ret.basic_blocks[brx_offset_first + i].brx_out.empty()) {
      ret.basic_blocks[brx_offset_first + i].instructions =
          cfg_first.basic_blocks[i].instructions;
      DEFINE_TERMINAL(brx_index_name, T_WORD,
                      ret.get_brx_index_name(brx_offset_first + i), true)
      ret.basic_blocks[brx_offset_first + i].brx_index = brx_index_name;
      GoPTX::Statement statement;
      GoPTX::VariableDeclarator vd;
      vd.id_or_opcode = brx_index_name;
      statement.variable_declarators.push_back(vd);
      statement.brefore_variable_declarator.assign({reg, u32});
      statement.instruction.instruction_aux.assign({t_semicolon});
      ret.identifier_decls.push_back(statement);
    }
  }
  for (size_t i = 0; i < cfg_second.basic_blocks.size(); ++i) {
    if (!ret.basic_blocks[brx_offset_second + i].brx_out.empty()) {
      ret.basic_blocks[brx_offset_second + i].instructions =
          cfg_second.basic_blocks[i].instructions;
      DEFINE_TERMINAL(brx_index_name, T_WORD,
                      ret.get_brx_index_name(brx_offset_second + i), true)
      ret.basic_blocks[brx_offset_second + i].brx_index = brx_index_name;
      GoPTX::Statement statement;
      GoPTX::VariableDeclarator vd;
      vd.id_or_opcode = brx_index_name;
      statement.variable_declarators.push_back(vd);
      statement.brefore_variable_declarator.assign({reg, u32});
      statement.instruction.instruction_aux.assign({t_semicolon});
      ret.identifier_decls.push_back(statement);
    }
  }
#endif
#if 1 // disable for debug
  ret.fuse_basic_block();
  ret.remove_dead_basic_block();
#endif
  return ret;
#undef DEFINE_TERMINAL
#endif
}

template struct GoPTX::StrategyMerge<GoPTX::BlockStrategyConcat>;
template struct GoPTX::StrategyMerge<GoPTX::BlockStrategyLatency>;

std::vector<GoPTX::Instruction> GoPTX::BlockStrategyConcat::operator()(
    const std::vector<GoPTX::Instruction> &block_first,
    const std::vector<GoPTX::Instruction> &block_second) const {
  std::vector<GoPTX::Instruction> ret;
  ret.insert(ret.end(), block_first.begin(), block_first.end());
  ret.insert(ret.end(), block_second.begin(), block_second.end());
  return ret;
}

int GoPTX::get_instruction_latency(const GoPTX::Instruction &ins,
                                   const std::string device_name) {
  assert(device_name == "a100pcie");
  // HPEC'22: Demystifying the Nvidia Ampere Architecture through
  // Microbenchmarking and Instruction-level Analysis
  const std::unordered_map<std::string, int> latency_table{
      {"alu_lat_double", 4},  {"alu_lat_float", 2},    {"alu_lat_half", 2},
      {"alu_lat_int32", 2},   {"Atomic_add_lat", 385}, {"l1_lat", 33},
      {"l2_lat", 200},        {"mem_lat", 290},        {"shared_lat", 23},
      {"tensor_lat_half", 16} // A100-40GB-PCIE
  };
  std::unordered_set<size_t> st;
  for (const auto &ia : ins.instruction_aux) {
    st.insert(ia.type);
  }
  if (st.count(GoPTX::Terminal::get_type_KI_ATOM())) {
    if (st.count(GoPTX::Terminal::get_type_K_GLOBAL()))
      return latency_table.at("Atomic_add_lat");
    if (st.count(GoPTX::Terminal::get_type_K_SHARED()))
      return latency_table.at("shared_lat");
  }
  if (st.count(GoPTX::Terminal::get_type_K_GLOBAL()))
    return latency_table.at("mem_lat");
  if (st.count(GoPTX::Terminal::get_type_K_SHARED()))
    return latency_table.at("shared_lat");
  if (st.count(GoPTX::Terminal::get_type_K_MMA()))
    return latency_table.at("tensor_lat_half");
  if (st.count(GoPTX::Terminal::get_type_KI_BAR()))
    return latency_table.at("shared_lat");
  return 4;
}

std::vector<GoPTX::Instruction> GoPTX::BlockStrategyLatency::operator()(
    const std::vector<GoPTX::Instruction> &block_first,
    const std::vector<GoPTX::Instruction> &block_second) const {
  std::vector<GoPTX::Instruction> ret;
#if 0
  GoPTX::BlockStrategyConcat tmp;
  ret = tmp(block_first, block_second);
  // ret = GoPTX::instruction_reorder(ret, block_first.size());
  return ret;
#else
  std::vector<int> latency_first(block_first.size()),
      latency_second(block_second.size());
  for (auto i = 0; i < block_first.size(); ++i)
    latency_first[i] = GoPTX::get_instruction_latency(block_first[i]);
  for (auto i = 0; i < block_second.size(); ++i)
    latency_second[i] = GoPTX::get_instruction_latency(block_second[i]);
  int p_first = 0, p_second = 0, sum_first = 0, sum_second = 0;
  if (!latency_first.empty() && !latency_second.empty()) {
    if (latency_second[0] < latency_first[0]) {
      sum_first += latency_first[p_first];
      ret.push_back(block_first[p_first]);
      ++p_first;
    } else {
      sum_second += latency_second[p_second];
      ret.push_back(block_second[p_second]);
      ++p_second;
    }
  }
  while (p_first < block_first.size() && p_second < block_second.size()) {
    if (sum_first < sum_second) {
      sum_first += latency_first[p_first];
      ret.push_back(block_first[p_first]);
      ++p_first;
    } else {
      sum_second += latency_second[p_second];
      ret.push_back(block_second[p_second]);
      ++p_second;
    }
  }
  while (p_first < block_first.size()) {
    ret.push_back(block_first[p_first]);
    ++p_first;
  }
  while (p_second < block_second.size()) {
    ret.push_back(block_second[p_second]);
    ++p_second;
  }
  return ret;
#endif
}