#pragma once
#ifdef __cplusplus

#include "GoPTX.h"
#include <string>
#include <vector>

namespace GoPTX {

struct Terminal;

struct Instruction;

struct VariableDeclarator;

struct Statement;

struct EntryOrFunc;

struct Prog;

struct ParseProgListener;

struct Terminal {
public:
  struct Equal {
    bool operator()(const Terminal &lhs, const Terminal &rhs) const;
  };
  std::string text;
  std::string to_string() const;
  size_t type;
  bool leading_space;
#ifdef GOPTX_DEF_GET_TYPE
#error
#else
#define GOPTX_DEF_GET_TYPE(W) static size_t get_type_##W();
  GOPTX_DEF_GET_TYPE(KI_ABS)
  GOPTX_DEF_GET_TYPE(KI_ADD)
  GOPTX_DEF_GET_TYPE(KI_ADDC)
  GOPTX_DEF_GET_TYPE(KI_AND)
  GOPTX_DEF_GET_TYPE(KI_ATOM)
  GOPTX_DEF_GET_TYPE(KI_BAR)
  GOPTX_DEF_GET_TYPE(KI_BARRIER)
  GOPTX_DEF_GET_TYPE(KI_BFE)
  GOPTX_DEF_GET_TYPE(KI_BFI)
  GOPTX_DEF_GET_TYPE(KI_BFIND)
  GOPTX_DEF_GET_TYPE(KI_BRA)
  GOPTX_DEF_GET_TYPE(KI_BREV)
  GOPTX_DEF_GET_TYPE(KI_BRKPT)
  GOPTX_DEF_GET_TYPE(KI_BRX)
  GOPTX_DEF_GET_TYPE(KI_CALL)
  GOPTX_DEF_GET_TYPE(KI_CLZ)
  GOPTX_DEF_GET_TYPE(KI_CNOT)
  GOPTX_DEF_GET_TYPE(KI_COPYSIGN)
  GOPTX_DEF_GET_TYPE(KI_COS)
  GOPTX_DEF_GET_TYPE(KI_CVT)
  GOPTX_DEF_GET_TYPE(KI_CVTA)
  GOPTX_DEF_GET_TYPE(KI_DIV)
  GOPTX_DEF_GET_TYPE(KI_EX2)
  GOPTX_DEF_GET_TYPE(KI_EXIT)
  GOPTX_DEF_GET_TYPE(KI_FMA)
  GOPTX_DEF_GET_TYPE(KI_ISSPACEP)
  GOPTX_DEF_GET_TYPE(KI_LD)
  GOPTX_DEF_GET_TYPE(KI_LDU)
  GOPTX_DEF_GET_TYPE(KI_LG2)
  GOPTX_DEF_GET_TYPE(KI_MAD)
  GOPTX_DEF_GET_TYPE(KI_MAD24)
  GOPTX_DEF_GET_TYPE(KI_MAX)
  GOPTX_DEF_GET_TYPE(KI_MEMBAR)
  GOPTX_DEF_GET_TYPE(KI_MIN)
  GOPTX_DEF_GET_TYPE(KI_MOV)
  GOPTX_DEF_GET_TYPE(KI_MUL)
  GOPTX_DEF_GET_TYPE(KI_MUL24)
  GOPTX_DEF_GET_TYPE(KI_NEG)
  GOPTX_DEF_GET_TYPE(KI_NOT)
  GOPTX_DEF_GET_TYPE(KI_OR)
  GOPTX_DEF_GET_TYPE(KI_PMEVENT)
  GOPTX_DEF_GET_TYPE(KI_POPC)
  GOPTX_DEF_GET_TYPE(KI_PREFETCH)
  GOPTX_DEF_GET_TYPE(KI_PREFETCHU)
  GOPTX_DEF_GET_TYPE(KI_PRMT)
  GOPTX_DEF_GET_TYPE(KI_RCP)
  GOPTX_DEF_GET_TYPE(KI_RED)
  GOPTX_DEF_GET_TYPE(KI_REM)
  GOPTX_DEF_GET_TYPE(KI_RET)
  GOPTX_DEF_GET_TYPE(KI_RSQRT)
  GOPTX_DEF_GET_TYPE(KI_SAD)
  GOPTX_DEF_GET_TYPE(KI_SELP)
  GOPTX_DEF_GET_TYPE(KI_SET)
  GOPTX_DEF_GET_TYPE(KI_SETP)
  GOPTX_DEF_GET_TYPE(KI_SHL)
  GOPTX_DEF_GET_TYPE(KI_SHR)
  GOPTX_DEF_GET_TYPE(KI_SIN)
  GOPTX_DEF_GET_TYPE(KI_SLCT)
  GOPTX_DEF_GET_TYPE(KI_SQRT)
  GOPTX_DEF_GET_TYPE(KI_ST)
  GOPTX_DEF_GET_TYPE(KI_SUB)
  GOPTX_DEF_GET_TYPE(KI_SUBC)
  GOPTX_DEF_GET_TYPE(KI_SULD)
  GOPTX_DEF_GET_TYPE(KI_SURED)
  GOPTX_DEF_GET_TYPE(KI_SUST)
  GOPTX_DEF_GET_TYPE(KI_SUQ)
  GOPTX_DEF_GET_TYPE(KI_TESTP)
  GOPTX_DEF_GET_TYPE(KI_TEX)
  GOPTX_DEF_GET_TYPE(KI_TXQ)
  GOPTX_DEF_GET_TYPE(KI_TRAP)
  GOPTX_DEF_GET_TYPE(KI_VABSDIFF)
  GOPTX_DEF_GET_TYPE(KI_VADD)
  GOPTX_DEF_GET_TYPE(KI_VMAD)
  GOPTX_DEF_GET_TYPE(KI_VMAX)
  GOPTX_DEF_GET_TYPE(KI_VMIN)
  GOPTX_DEF_GET_TYPE(KI_VSET)
  GOPTX_DEF_GET_TYPE(KI_VSHL)
  GOPTX_DEF_GET_TYPE(KI_VSHR)
  GOPTX_DEF_GET_TYPE(KI_VSUB)
  GOPTX_DEF_GET_TYPE(KI_VOTE)
  GOPTX_DEF_GET_TYPE(KI_XOR)
  GOPTX_DEF_GET_TYPE(KI_WMMA)
  GOPTX_DEF_GET_TYPE(K_ALIGNED)
  GOPTX_DEF_GET_TYPE(K_B16)
  GOPTX_DEF_GET_TYPE(K_B32)
  GOPTX_DEF_GET_TYPE(K_B64)
  GOPTX_DEF_GET_TYPE(K_B8)
  GOPTX_DEF_GET_TYPE(K_BRANCHTARGETS)
  GOPTX_DEF_GET_TYPE(K_GLOBAL)
  GOPTX_DEF_GET_TYPE(K_F16)
  GOPTX_DEF_GET_TYPE(K_F32)
  GOPTX_DEF_GET_TYPE(K_F64)
  GOPTX_DEF_GET_TYPE(K_IDX)
  GOPTX_DEF_GET_TYPE(K_LO)
  GOPTX_DEF_GET_TYPE(K_LOAD)
  GOPTX_DEF_GET_TYPE(K_MMA)
  GOPTX_DEF_GET_TYPE(K_NE)
  GOPTX_DEF_GET_TYPE(K_PRED)
  GOPTX_DEF_GET_TYPE(K_REG)
  GOPTX_DEF_GET_TYPE(K_S16)
  GOPTX_DEF_GET_TYPE(K_S32)
  GOPTX_DEF_GET_TYPE(K_S64)
  GOPTX_DEF_GET_TYPE(K_S8)
  GOPTX_DEF_GET_TYPE(K_SHARED)
  GOPTX_DEF_GET_TYPE(K_STORE)
  GOPTX_DEF_GET_TYPE(K_SYNC)
  GOPTX_DEF_GET_TYPE(K_U16)
  GOPTX_DEF_GET_TYPE(K_U32)
  GOPTX_DEF_GET_TYPE(K_U64)
  GOPTX_DEF_GET_TYPE(K_U8)
  GOPTX_DEF_GET_TYPE(K_UNI)
  GOPTX_DEF_GET_TYPE(K_X)
  GOPTX_DEF_GET_TYPE(K_Y)
  GOPTX_DEF_GET_TYPE(K_Z)
  GOPTX_DEF_GET_TYPE(T_AT)
  GOPTX_DEF_GET_TYPE(T_COLON)
  GOPTX_DEF_GET_TYPE(T_COMMA)
  GOPTX_DEF_GET_TYPE(T_DEC_LITERAL)
  GOPTX_DEF_GET_TYPE(T_NOT)
  GOPTX_DEF_GET_TYPE(T_SEMICOLON)
  GOPTX_DEF_GET_TYPE(T_WORD)
#undef GOPTX_DEF_GET_TYPE
#endif
  std::pair<int, int> match_label_decl_or_variable_declarator(
      const std::vector<Statement> &statements) const;
  std::pair<int, int>
  match_param(const std::vector<std::vector<Terminal>> &param_list) const;
};

struct Instruction {
  std::vector<Terminal> predicate, instruction_aux;
};

int get_instruction_latency(const Instruction &ins,
                            const std::string device_name = "a100pcie");

struct VariableDeclarator {
public:
  std::string to_string() const;
  Terminal id_or_opcode;
  std::vector<Terminal> parameterized_register_spec, array_spec,
      equal_initializer;
};

struct Statement {
public:
  std::string to_string() const;

  std::vector<Terminal> branch_targets, label_decl, i_bra, i_ret,
      brefore_variable_declarator;
  std::vector<VariableDeclarator> variable_declarators;
  Instruction instruction;
};

struct ControlFlowGraph {
  struct BasicBlock {
    std::vector<size_t> out, brx_out;
    Terminal bra_predicate, brx_index;
    std::vector<Instruction> instructions;
  };
  ControlFlowGraph(const std::vector<Statement> &statements);
  std::string to_mermaid() const;
  std::vector<Statement>
  to_statements(const std::string &label_suffix,
                const std::string &label_prefix = "$") const;
  void replace_aligned();
  void split_aligned_block();
  void split_basic_block_by_latency();
  void fuse_basic_block();
  void remove_dead_basic_block();
  std::string get_brx_index_name(size_t index) const;
  std::vector<int8_t> get_has_keyword(size_t keyword) const;
  std::vector<int8_t> get_before_sync() const;
  std::vector<BasicBlock> basic_blocks;
  std::vector<Statement> identifier_decls;
};

struct EntryOrFunc {
public:
  std::string to_string() const;
  void add_suffix(const std::string &suffix);
  std::vector<Statement> statements;
  Terminal name, k_entry;
  std::vector<std::vector<Terminal>> param_list;
  std::vector<std::string> get_identifiers() const;
  Terminal k_visible, k_extern, k_func;
  std::vector<std::vector<Terminal>> func_ret_list,
      performance_tuning_directives;
};

struct Prog {
public:
  std::string to_string() const;
  int concat(Prog prog_second, const std::string &suffix_first,
             const std::string &suffix_second);
  std::vector<EntryOrFunc> entry_or_func_list;
  std::vector<std::string> get_identifiers() const;
  void add_suffix(const std::string &suffix);
  std::vector<Terminal> version, target, address_size;
  std::vector<Statement> statements;
};

template <typename Listener = ParseProgListener> Prog parse(const char *ptx);

extern template Prog parse<>(const char *ptx);

struct StrategyVFuse {
  ControlFlowGraph operator()(const ControlFlowGraph &cfg_first,
                              const ControlFlowGraph &cfg_second,
                              const GoPTX_config &config) const;
};

struct StrategyHFuse {
  ControlFlowGraph operator()(const ControlFlowGraph &cfg_first,
                              const ControlFlowGraph &cfg_second,
                              const GoPTX_config &config) const;
};

struct BlockStrategyConcat {
  std::vector<Instruction>
  operator()(const std::vector<Instruction> &block_first,
             const std::vector<Instruction> &block_second) const;
};
struct BlockStrategyLatency {
  std::vector<Instruction>
  operator()(const std::vector<Instruction> &block_first,
             const std::vector<Instruction> &block_second) const;
};

template <typename BlockStrategy = BlockStrategyLatency> struct StrategyMerge {
  ControlFlowGraph operator()(const ControlFlowGraph &cfg_first,
                              const ControlFlowGraph &cfg_second,
                              const GoPTX_config &config) const;
};

extern template struct StrategyMerge<BlockStrategyConcat>;
extern template struct StrategyMerge<BlockStrategyLatency>;

} // namespace GoPTX

#endif