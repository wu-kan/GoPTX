#include "GoPTX/GoPTX.hh"
#include <PtxBaseListener.h>
#include <PtxLexer.h>
#include <PtxParser.h>
#include <algorithm>
#include <cassert>
#include <initializer_list>
#include <string>
#ifdef GOPTX_DEF_GET_TYPE
#error
#else
#define GOPTX_DEF_GET_TYPE(W)                                                  \
  size_t GoPTX::Terminal::get_type_##W() { return GoPTX::PtxLexer::W; }
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

template <typename Listener> GoPTX::Prog GoPTX::parse(const char *ptx) {
  try {
    antlr4::ANTLRInputStream input(ptx, std::strlen(ptx));
    GoPTX::PtxLexer lexer(&input);
    antlr4::CommonTokenStream tokens(&lexer);
    GoPTX::PtxParser parser(&tokens);
    parser.setErrorHandler(std::make_shared<antlr4::BailErrorStrategy>());
    Listener listener;
    antlr4::tree::ParseTreeWalker::DEFAULT.walk(&listener, parser.prog());
    return listener.prog;
  } catch (const antlr4::ParseCancellationException &e) {
    std::fprintf(stderr,
                 "{\"File\": \"%s\", \"Line\": %d, \"Msg\": \"%s. %s\"}\n",
                 __FILE__, __LINE__,
                 "error: parser error, maybe unsupported PTX grammar, you "
                 "should update `Ptx.g4`",
                 e.what());
    std::exit(-1);
  }
}

struct GoPTX::ParseProgListener : GoPTX::PtxBaseListener {
  GoPTX::Prog prog;
  std::vector<GoPTX::Terminal> terminal_flow;
  std::vector<size_t> rule_stack;
  std::vector<decltype(terminal_flow.size())> flow_stack;
  bool rule_stack_find(
      const std::initializer_list<decltype(GoPTX::PtxParser::RuleProg)> &rules)
      const {
    return std::find_first_of(rule_stack.begin(), rule_stack.end(),
                              rules.begin(), rules.end()) != rule_stack.end();
  }
  bool terminal_type_find(const std::initializer_list<size_t> &types,
                          size_t terminal_type) const {
    return std::find(types.begin(), types.end(), terminal_type) != types.end();
  }
  GoPTX::Terminal transform_terminal(antlr4::tree::TerminalNode &tn) const {
    auto symbol = tn.getSymbol();
    auto type = symbol->getType();
    GoPTX::Terminal terminal;
    terminal.text = symbol->getText();
    terminal.type = type;
    terminal.leading_space = false;
    return terminal;
  }
  size_t unterminated_directive_cnt = 0;
  std::vector<std::pair<size_t, std::vector<GoPTX::Statement>>>
      unterminated_directive_stak;
  void enterEveryRule(antlr4::ParserRuleContext *ctx) override {
    if (ctx->getRuleIndex() == GoPTX::PtxParser::RuleUnterminated_directive) {
      if (!unterminated_directive_stak.empty()) {
        std::fprintf(stderr,
                     "{\"File\": \"%s\", \"Line\": %d, \"Msg\": \"%s\"}\n",
                     __FILE__, __LINE__,
                     "warn: Find nested braces, please check the results. "
                     "Suffix may be added.");
      }
      unterminated_directive_stak.emplace_back(++unterminated_directive_cnt,
                                               std::vector<GoPTX::Statement>());
    }
    rule_stack.push_back(ctx->getRuleIndex());
    flow_stack.push_back(terminal_flow.size());
  }
  void exitEveryRule(antlr4::ParserRuleContext *ctx) override {
    rule_stack.pop_back();
    flow_stack.pop_back();
    if (ctx->getRuleIndex() == GoPTX::PtxParser::RuleUnterminated_directive) {
      unterminated_directive_stak.pop_back();
    }
  }
  void visitTerminal(antlr4::tree::TerminalNode *node) override {
    GoPTX::Terminal terminal = transform_terminal(*node);
    terminal_flow.push_back(terminal);
  }
  void enterControl_flow_directive(
      GoPTX::PtxParser::Control_flow_directiveContext *ctx) override {
    std::fprintf(stderr, "{\"File\": \"%s\", \"Line\": %d, \"Msg\": \"%s\"}\n",
                 __FILE__, __LINE__,
                 "error: GoPTX does not support control_flow_directive now.");
    std::exit(-1);
  }
#ifdef GOPTX_ADD_PREFIX
#error
#else
#define GOPTX_ADD_PREFIX(op)                                                   \
  void exit##op(GoPTX::PtxParser::op##Context *ctx) override {                 \
    terminal_flow[flow_stack.back()].leading_space = true;                     \
  }
  GOPTX_ADD_PREFIX(Opr)
  GOPTX_ADD_PREFIX(Opr_label)
  GOPTX_ADD_PREFIX(Opr_register)
  GOPTX_ADD_PREFIX(Opr_register_or_constant)
#undef GOPTX_ADD_PREFIX
#endif
  void exitVersion(GoPTX::PtxParser::VersionContext *ctx) override {
    (terminal_flow.begin() + flow_stack.back() + 1)->leading_space = true;
    prog.version.assign(terminal_flow.begin() + flow_stack.back(),
                        terminal_flow.end());
  }
  void exitTarget(GoPTX::PtxParser::TargetContext *ctx) override {
    (terminal_flow.begin() + flow_stack.back() + 1)->leading_space = true;
    prog.target.assign(terminal_flow.begin() + flow_stack.back(),
                       terminal_flow.end());
  }
  void exitAddress_size(GoPTX::PtxParser::Address_sizeContext *ctx) override {
    (terminal_flow.begin() + flow_stack.back() + 1)->leading_space = true;
    prog.address_size.assign(terminal_flow.begin() + flow_stack.back(),
                             terminal_flow.end());
  }
  void enterEntry(GoPTX::PtxParser::EntryContext *ctx) override {
    GoPTX::EntryOrFunc entry;
    prog.entry_or_func_list.push_back(entry);
  }
  void enterFunc(GoPTX::PtxParser::FuncContext *ctx) override {
    GoPTX::EntryOrFunc func;
    prog.entry_or_func_list.push_back(func);
  }
  void exitEntry_aux(GoPTX::PtxParser::Entry_auxContext *ctx) override {
    if (ctx->K_ENTRY() != nullptr) {
      prog.entry_or_func_list.back().k_entry =
          transform_terminal(*(ctx->K_ENTRY()));
    }
    if (ctx->K_VISIBLE() != nullptr) {
      prog.entry_or_func_list.back().k_visible =
          transform_terminal(*(ctx->K_VISIBLE()));
    }
    if (ctx->K_EXTERN() != nullptr) {
      prog.entry_or_func_list.back().k_extern =
          transform_terminal(*(ctx->K_EXTERN()));
    }
  }
  void exitFunc_aux(GoPTX::PtxParser::Func_auxContext *ctx) override {
    if (ctx->K_FUNC() != nullptr) {
      prog.entry_or_func_list.back().k_func =
          transform_terminal(*(ctx->K_FUNC()));
    }
    if (ctx->K_VISIBLE() != nullptr) {
      prog.entry_or_func_list.back().k_visible =
          transform_terminal(*(ctx->K_VISIBLE()));
    }
    if (ctx->K_EXTERN() != nullptr) {
      prog.entry_or_func_list.back().k_extern =
          transform_terminal(*(ctx->K_EXTERN()));
    }
  }
  void exitKernel_name(GoPTX::PtxParser::Kernel_nameContext *ctx) override {
    terminal_flow.back().leading_space = true;
    prog.entry_or_func_list.back().name = terminal_flow.back();
  }
  void exitFunc_name(GoPTX::PtxParser::Func_nameContext *ctx) override {
    terminal_flow.back().leading_space = true;
    prog.entry_or_func_list.back().name = terminal_flow.back();
  }
  void exitEntry_param(GoPTX::PtxParser::Entry_paramContext *ctx) override {
    for (auto it = terminal_flow.begin() + flow_stack.back(),
              end = terminal_flow.end();
         it != end; ++it) {
      it->leading_space = true;
    }
    prog.entry_or_func_list.back().param_list.emplace_back(
        terminal_flow.begin() + flow_stack.back(), terminal_flow.end());
  }
  void exitFunc_param(GoPTX::PtxParser::Func_paramContext *ctx) override {
    for (auto it = terminal_flow.begin() + flow_stack.back(),
              end = terminal_flow.end();
         it != end; ++it) {
      it->leading_space = true;
    }
    prog.entry_or_func_list.back().param_list.emplace_back(
        terminal_flow.begin() + flow_stack.back(), terminal_flow.end());
  }
  void exitFunc_ret(GoPTX::PtxParser::Func_retContext *ctx) override {
    for (auto it = terminal_flow.begin() + flow_stack.back(),
              end = terminal_flow.end();
         it != end; ++it) {
      it->leading_space = true;
    }
    prog.entry_or_func_list.back().func_ret_list.emplace_back(
        terminal_flow.begin() + flow_stack.back(), terminal_flow.end());
  }
  void exitPerformance_tuning_directive(
      GoPTX::PtxParser::Performance_tuning_directiveContext *ctx) override {
    prog.entry_or_func_list.back().performance_tuning_directives.emplace_back(
        terminal_flow.begin() + flow_stack.back(), terminal_flow.end());
  }
  std::vector<GoPTX::Statement> statement_stack;
  void enterStatement(GoPTX::PtxParser::StatementContext *ctx) override {
    GoPTX::Statement statement;
    statement_stack.push_back(statement);
  }
  void exitStatement(GoPTX::PtxParser::StatementContext *ctx) override {
    bool is_pragma = false;
    auto ctx1 = ctx->semicolon_terminated_statement();
    if (ctx1 != nullptr && ctx1->pragma() != nullptr)
      is_pragma = true;
    bool is_unterminated_statement = ctx->unterminated_statement() != nullptr;
    if (!is_pragma && !is_unterminated_statement) {
      auto statement = statement_stack.back();
      if (statement.brefore_variable_declarator.empty() &&
          statement.variable_declarators.empty()) {
        statement.instruction.instruction_aux.assign(
            terminal_flow.begin() + flow_stack.back() +
                statement.label_decl.size() +
                statement.instruction.predicate.size() +
                statement.i_bra.size() + statement.i_ret.size(),
            terminal_flow.end());
      } else {
        if (terminal_flow.back().type == GoPTX::PtxLexer::T_SEMICOLON &&
            (statement.instruction.instruction_aux.empty() ||
             statement.instruction.instruction_aux.back().type !=
                 GoPTX::PtxLexer::T_SEMICOLON))
          statement.instruction.instruction_aux.push_back(terminal_flow.back());
      }
      if (!statement.instruction.predicate.empty()) {
        if (!statement.instruction.instruction_aux.empty())
          statement.instruction.instruction_aux.front().leading_space = true;
        if (!statement.i_bra.empty())
          statement.i_bra.front().leading_space = true;
        if (!statement.i_ret.empty())
          statement.i_ret.front().leading_space = true;
      }
      if (unterminated_directive_stak.size() >
          1) { // do not consider the outer braces
        for (auto &variable_declarator : statement.variable_declarators) {
          const auto suffix =
              "__GoPTX_braces_" +
              std::to_string(unterminated_directive_stak.back().first) + "__";
          variable_declarator.id_or_opcode.text += suffix;
        }
        for (const auto &pwords : {&(statement.instruction.predicate),
                                   &(statement.instruction.instruction_aux)}) {
          auto &words = *pwords;
          for (auto &terminal : words) {
            for (int i = (int)unterminated_directive_stak.size() - 1; i >= 1;
                 --i) { // do not consider the outer braces
              const auto suffix =
                  "__GoPTX_braces_" +
                  std::to_string(unterminated_directive_stak[i].first) + "__";
              auto &statements = unterminated_directive_stak[i].second;
              auto it =
                  terminal.match_label_decl_or_variable_declarator(statements);
              if (it.first >= 0) {
                if (it.second >= 0) {
                  if (!statements[it.first]
                           .variable_declarators[it.second]
                           .parameterized_register_spec.empty()) {
                    terminal.text = statements[it.first]
                                        .variable_declarators[it.second]
                                        .id_or_opcode.text +
                                    suffix +
                                    terminal.text.substr(
                                        statements[it.first]
                                            .variable_declarators[it.second]
                                            .id_or_opcode.text.size());
                  } else {
                    terminal.text += suffix;
                  }
                } else {
                  terminal.text += suffix;
                }
                break;
              }
            }
          }
        }
      }
      if (rule_stack_find({GoPTX::PtxParser::RuleEntry_body,
                           GoPTX::PtxParser::RuleFunc_body})) {
        prog.entry_or_func_list.back().statements.push_back(statement);
      } else {
        prog.statements.push_back(statement);
      }
    }
    statement_stack.pop_back();
  }
  void enterLabel_decl(GoPTX::PtxParser::Label_declContext *ctx) override {
    if (unterminated_directive_stak.size() != 1) {
      std::fprintf(stderr,
                   "{\"File\": \"%s\", \"Line\": %d, \"Msg\": \"%s\"}\n",
                   __FILE__, __LINE__,
                   "error: not expected label_decls, maybe not in braces or in "
                   "nested braces. GoPTX does not consider this now.");
      std::exit(-1);
    }
  }
  void exitLabel_decl(GoPTX::PtxParser::Label_declContext *ctx) override {
    statement_stack.back().label_decl.assign(
        terminal_flow.begin() + flow_stack.back(), terminal_flow.end());
  }
  void exitPredicate(GoPTX::PtxParser::PredicateContext *ctx) override {
    statement_stack.back().instruction.predicate.assign(
        terminal_flow.begin() + flow_stack.back(), terminal_flow.end());
  }
  void exitI_bra(GoPTX::PtxParser::I_braContext *ctx) override {
    statement_stack.back().i_bra.assign(
        terminal_flow.begin() + flow_stack.back(), terminal_flow.end());
  }
  void exitI_ret(GoPTX::PtxParser::I_retContext *ctx) override {
    statement_stack.back().i_ret.assign(
        terminal_flow.begin() + flow_stack.back(), terminal_flow.end());
  }
  void exitLinking_directive(
      GoPTX::PtxParser::Linking_directiveContext *ctx) override {
    statement_stack.back().brefore_variable_declarator.insert(
        statement_stack.back().brefore_variable_declarator.begin(),
        terminal_flow[flow_stack.back()]);
  }
  size_t end_of_brefore_variable_declarator;
  void enterIdentifier_decl_aux(
      GoPTX::PtxParser::Identifier_decl_auxContext *ctx) override {
    end_of_brefore_variable_declarator = flow_stack.back();
  }
  void exitIdentifier_decl_aux(
      GoPTX::PtxParser::Identifier_decl_auxContext *ctx) override {
    terminal_flow[flow_stack.back()].leading_space = true;
    statement_stack.back().brefore_variable_declarator.assign(
        terminal_flow.begin() + flow_stack.back(),
        terminal_flow.begin() + end_of_brefore_variable_declarator);
    if (!unterminated_directive_stak.empty()) {
      unterminated_directive_stak.back().second.push_back(
          statement_stack.back());
    }
  }
  void exitType(GoPTX::PtxParser::TypeContext *ctx) override {
    terminal_flow[flow_stack.back()].leading_space = true;
    end_of_brefore_variable_declarator =
        std::max(end_of_brefore_variable_declarator, terminal_flow.size());
  }
  void exitTexref(GoPTX::PtxParser::TexrefContext *ctx) override {
    end_of_brefore_variable_declarator =
        std::max(end_of_brefore_variable_declarator, terminal_flow.size());
  }
  void exitAlign(GoPTX::PtxParser::AlignContext *ctx) override {
    end_of_brefore_variable_declarator =
        std::max(end_of_brefore_variable_declarator, terminal_flow.size());
    terminal_flow.back().leading_space = true;
  }
  void enterVariable_declarator(
      GoPTX::PtxParser::Variable_declaratorContext *ctx) override {
    GoPTX::VariableDeclarator variable_declarator;
    statement_stack.back().variable_declarators.push_back(variable_declarator);
  }
  void exitId_or_opcode(GoPTX::PtxParser::Id_or_opcodeContext *ctx) override {
    terminal_flow[flow_stack.back()].leading_space = true;
    if (rule_stack_find({GoPTX::PtxParser::RuleVariable_declarator}) &&
        !rule_stack_find({GoPTX::PtxParser::RuleEqual_initializer})) {
      statement_stack.back().variable_declarators.back().id_or_opcode =
          terminal_flow[flow_stack.back()];
      assert(flow_stack.back() == terminal_flow.size() - 1);
    }
  }
  void exitParameterized_register_spec(
      GoPTX::PtxParser::Parameterized_register_specContext *ctx) override {
    if (rule_stack_find({GoPTX::PtxParser::RuleVariable_declarator})) {
      statement_stack.back()
          .variable_declarators.back()
          .parameterized_register_spec.assign(
              terminal_flow.begin() + flow_stack.back(), terminal_flow.end());
    }
  }
  void exitArray_spec(GoPTX::PtxParser::Array_specContext *ctx) override {
    if (rule_stack_find({GoPTX::PtxParser::RuleVariable_declarator})) {
      statement_stack.back().variable_declarators.back().array_spec.assign(
          terminal_flow.begin() + flow_stack.back(), terminal_flow.end());
    }
  }
  void exitEqual_initializer(
      GoPTX::PtxParser::Equal_initializerContext *ctx) override {
    if (rule_stack_find({GoPTX::PtxParser::RuleVariable_declarator})) {
      statement_stack.back()
          .variable_declarators.back()
          .equal_initializer.assign(terminal_flow.begin() + flow_stack.back(),
                                    terminal_flow.end());
    }
  }
};

template GoPTX::Prog GoPTX::parse<GoPTX::ParseProgListener>(const char *ptx);
