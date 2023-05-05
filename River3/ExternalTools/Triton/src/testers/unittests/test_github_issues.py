#!/usr/bin/env python
# coding: utf-8
"""Issue from Github."""

import unittest

from triton import *



class TestIssue656(unittest.TestCase):

    """Testing #656."""

    @staticmethod
    def sym_exec_gadget(gadget):
        ctx = TritonContext()
        ctx.setArchitecture(ARCH.X86_64)
        ctx.setMode(MODE.ALIGNED_MEMORY, True)

        r = ctx.registers
        gprs = (
            r.rax, r.rbx, r.rcx, r.rdx,
            r.rsi, r.rdi, r.esp, r.ebp,
            r.r8, r.r9, r.r10, r.r11,
            r.r12, r.r13, r.r14
        )

        for gpr in gprs:
            ctx.symbolizeRegister(gpr)

        for assembly in gadget:
            i = Instruction()
            i.setOpcode(assembly)
            ctx.processing(i)

        srax = ctx.getSymbolicRegister(ctx.registers.rax)

        return srax


    def test_issue(self):
        e = self.sym_exec_gadget((b'\x89\xd8', b'\xc2\x04\x00'))
        self.assertEqual(str(e), '(define-fun ref!15 () (_ BitVec 64) ((_ zero_extend 32) ((_ extract 31 0) ref!1))) ; MOV operation')
        a = e.getAst()
        self.assertEqual(str(a + 1), '(bvadd ((_ zero_extend 32) ((_ extract 31 0) ref!1)) (_ bv1 64))')
        self.assertEqual(str(1 + a), '(bvadd (_ bv1 64) ((_ zero_extend 32) ((_ extract 31 0) ref!1)))')
        self.assertEqual(str(e.getComment()), 'MOV operation')
        self.assertEqual(e.getId(), 15)
        self.assertEqual(str(e.getAst()), str(e.getNewAst()))
        self.assertEqual(str(e.getOrigin()), 'rax:64 bv[63..0]')
        self.assertEqual(e.getType(), SYMBOLIC.REGISTER_EXPRESSION)
        self.assertEqual(e.isMemory(), False)
        self.assertEqual(e.isRegister(), True)
        self.assertEqual(e.isSymbolized(), True)
        e.setComment('test')
        self.assertEqual(str(e.getComment()), 'test')
        n = e.getNewAst() + 1
        e.setAst(n)
        self.assertEqual(str(e.getAst()), str(n))



class TestIssue673(unittest.TestCase):

    """Testing #673."""

    def setUp(self):
        """Define the arch."""
        self.ctx = TritonContext()
        self.ctx.setArchitecture(ARCH.X86_64)


    def test_issue1(self):
        inst = Instruction(b"\xc0\xc0\x00") # rol al, 0
        self.ctx.processing(inst)
        self.assertEqual(len(inst.getUndefinedRegisters()), 0)
        self.assertEqual(len(inst.getReadRegisters()), 1)
        self.assertEqual(len(inst.getWrittenRegisters()), 2)


    def test_issue2(self):
        inst = Instruction(b"\xc0\xc0\x01") # rol al, 1
        self.ctx.processing(inst)
        self.assertEqual(len(inst.getUndefinedRegisters()), 0)
        self.assertEqual(len(inst.getReadRegisters()), 2)
        self.assertEqual(len(inst.getWrittenRegisters()), 4)


    def test_issue3(self):
        inst = Instruction(b"\xc0\xc0\x07") # rol al, 7
        self.ctx.processing(inst)
        self.assertEqual(len(inst.getUndefinedRegisters()), 1)
        self.assertEqual(len(inst.getReadRegisters()), 2)
        self.assertEqual(len(inst.getWrittenRegisters()), 4)



class TestIssue792(unittest.TestCase):

    """Testing #792."""

    def setUp(self):
        """Define the arch."""
        self.ctx = TritonContext()
        self.ctx.setArchitecture(ARCH.X86_64)


    def test_issue(self):
        ac = self.ctx.getAstContext()

        var1 = self.ctx.newSymbolicVariable(64, 'var1')
        var2 = self.ctx.newSymbolicVariable(64, 'var2')

        ast_original  = ac.bvadd(ac.variable(var1), ac.variable(var2))
        ast_duplicate = ac.duplicate(ast_original)
        ast_unrolled  = ac.unroll(ast_original)

        self.ctx.setConcreteVariableValue(var1, 4)
        self.ctx.setConcreteVariableValue(var2, 2)

        self.assertEqual(ast_original.evaluate(), 6)
        self.assertEqual(ast_duplicate.evaluate(), 6)
        self.assertEqual(ast_unrolled.evaluate(), 6)

        ast_original.setChild(0, ac.bv(1, 64))

        self.assertEqual(ast_original.evaluate(), 3)
        self.assertEqual(ast_duplicate.evaluate(), 6)
        self.assertEqual(ast_unrolled.evaluate(), 6)

        ast_duplicate.setChild(0, ac.bv(10, 64))

        self.assertEqual(ast_original.evaluate(), 3)
        self.assertEqual(ast_duplicate.evaluate(), 12)
        self.assertEqual(ast_unrolled.evaluate(), 6)



class TestIssue795(unittest.TestCase):

    """Testing #795."""

    def setUp(self):
        """Define the arch."""
        self.ctx = TritonContext()
        self.ctx.setArchitecture(ARCH.X86_64)


    def test_issue(self):
        ast = self.ctx.getAstContext()

        var1    = self.ctx.newSymbolicVariable(64, 'var1')
        var2    = self.ctx.newSymbolicVariable(64, 'var2')
        var1ast = ast.variable(var1)
        var2ast = ast.variable(var2)

        a1 = ast.bvadd(var1ast, var2ast)
        b1 = ast.bvnot(a1)
        b2 = ast.duplicate(b1)

        self.assertEqual(len(a1.getParents()), 1)
        self.assertEqual(len(b2.getParents()), 0)
        self.assertEqual(len(b2.getChildren()[0].getParents()), 1)
        self.assertEqual(len(var1ast.getParents()), 2)
        self.assertEqual(len(var2ast.getParents()), 2)

        self.assertEqual(b1.evaluate(), b2.evaluate())
        self.ctx.setConcreteVariableValue(var1, 4)
        self.ctx.setConcreteVariableValue(var2, 2)
        self.assertEqual(b1.evaluate(), b2.evaluate())



class TestIssue789(unittest.TestCase):

    """Testing #789."""

    def setUp(self):
        self.ctx = TritonContext()
        self.ctx.setArchitecture(ARCH.X86_64)
        self.ctx.setMode(MODE.ALIGNED_MEMORY, True)

        self.mem_symvars = []
        self.reg_symvars = []
        self.seen_regs   = []
        self.seen_mem    = []
        self.code        = [
            b"\x48\x8b\x1d\x00\x01\x00\x00",    # mov rbx, [0x100]
            b"\x48\x01\xd8",                    # add rax, rbx
        ]


    def handle_mem_read(self, ctx, ma):
        # Keep track of seen registers to avoid recursion
        addr = ma.getAddress()
        if addr in self.seen_mem: return
        self.seen_mem.append(addr)

        # Create symbolic var
        alias = "mem_{:#x}".format(ma.getAddress())
        symvar = ctx.symbolizeMemory(ma, alias)
        self.mem_symvars.append(symvar)


    def handle_reg_read(self, ctx, reg):
        # Keep track of seen registers to avoid recursion
        reg_id = reg.getId()
        if reg_id in self.seen_regs: return
        self.seen_regs.append(reg_id)

        # Create symbolic var
        alias = "sym_reg_{}".format(reg.getName())
        symvar = ctx.symbolizeRegister(reg, alias)
        self.reg_symvars.append(symvar)


    def emulate(self, ip):
        for opcode in self.code:
            inst = Instruction(opcode)
            inst.setAddress(ip)
            self.ctx.processing(inst)
            ip = self.ctx.getRegisterAst(self.ctx.registers.rip).evaluate()


    def test_issue(self):
        self.ctx.addCallback(self.handle_mem_read, CALLBACK.GET_CONCRETE_MEMORY_VALUE)
        self.ctx.addCallback(self.handle_reg_read, CALLBACK.GET_CONCRETE_REGISTER_VALUE)
        self.emulate(0x400000)
        ast = self.ctx.getAstContext()
        rax = self.ctx.getSymbolicRegister(self.ctx.registers.rax)
        self.assertEqual(str(ast.unroll(rax.getAst())), "(bvadd sym_reg_rax sym_reg_rbx)")



class TestIssue803(unittest.TestCase):

    """Testing #803."""

    def setUp(self):
        self.ctx = TritonContext()
        self.ctx.setArchitecture(ARCH.X86_64)
        self.ast = self.ctx.getAstContext()


    def test_issue(self):
        # Create two different symbolic variables
        other_vars = [self.ctx.newSymbolicVariable(64, 'myvar1') for _ in range(4)] # Unused vars (ignore this)
        var1 = self.ctx.newSymbolicVariable(64, 'myvar1')
        other_vars2 = [self.ctx.newSymbolicVariable(64, 'myvar1') for _ in range(3)] # Unused vars (ingore this)
        var2 = self.ctx.newSymbolicVariable(64, 'myvar2')

        # Create two different variable nodes
        ast1 = self.ast.variable(var1)
        ast2 = self.ast.variable(var2)

        # Make variables have the same value (optional)
        self.ctx.setConcreteVariableValue(var1, 42)
        self.ctx.setConcreteVariableValue(var2, 42)

        # Test
        self.assertFalse(ast1.equalTo(ast2))



class TestIssue820(unittest.TestCase):

    """Testing #820."""

    def setUp(self):
        self.ctx = TritonContext()


    def test_issue1(self):
        self.ctx.setArchitecture(ARCH.X86_64)
        self.ast = self.ctx.getAstContext()

        code = [
            b"\x48\xff\xc0", # inc rax
            b"\x48\xff\xc0", # inc rax
            b"\x48\xff\xc0", # inc rax
        ]

        for op in code:
            inst = Instruction(op)
            self.ctx.processing(inst)

        rax = self.ctx.getSymbolicRegister(self.ctx.registers.rax)
        self.assertEqual(rax.getAst().evaluate(), 3)

        # Testing initParents() when setting an AST on a old symbolic expression
        ref0 = self.ctx.getSymbolicExpression(0)
        ref0.setAst(self.ast.bv(10, 64))
        self.assertEqual(rax.getAst().evaluate(), 12)


    def test_issue2(self):
        self.ctx.setArchitecture(ARCH.X86_64)
        self.ast = self.ctx.getAstContext()

        code = [
            b"\x48\xff\xc0", # inc rax
            b"\x48\xff\xc0", # inc rax
            b"\x48\xff\xc0", # inc rax
        ]

        for op in code:
            inst = Instruction(op)
            self.ctx.processing(inst)

        rax = self.ctx.getSymbolicRegister(self.ctx.registers.rax)
        self.assertEqual(rax.getAst().evaluate(), 3)

        # Testing initParents() when setting the child of an old AST
        ref0 = self.ctx.getSymbolicExpression(0)
        ref0.getAst().setChild(0, self.ast.bv(10, 64))
        self.assertEqual(rax.getAst().evaluate(), 13)



class TestIssue818(unittest.TestCase):

    """Testing #818."""

    def setUp(self):
        self.ctx = TritonContext()
        self.ctx.setArchitecture(ARCH.X86_64)
        self.ast = self.ctx.getAstContext()


    def test_issue1(self):
        var1 = self.ctx.symbolizeRegister(self.ctx.registers.al)
        var2 = self.ctx.symbolizeRegister(self.ctx.registers.ah)
        v1 = self.ast.variable(var1)
        v2 = self.ast.variable(var2)
        rax = self.ctx.getSymbolicRegister(self.ctx.registers.rax)
        self.assertEqual(str(self.ast.unroll(rax.getAst())), '(concat ((_ extract 63 16) (concat ((_ extract 63 8) (_ bv0 64)) SymVar_0)) (concat SymVar_1 ((_ extract 7 0) (concat ((_ extract 63 8) (_ bv0 64)) SymVar_0))))')

    def test_issue2(self):
        var1 = self.ctx.symbolizeRegister(self.ctx.registers.al)
        var2 = self.ctx.symbolizeRegister(self.ctx.registers.ah)

        inst = Instruction(b"\xff\xc0") # inc rax
        self.ctx.processing(inst)

        ref1 = self.ctx.getSymbolicExpression(2) # res of 'inc rax'
        m = self.ctx.getModel(ref1.getAst() == 0xdead)
        self.assertEqual(m[0].getValue(), 0xac)
        self.assertEqual(m[1].getValue(), 0xde)


class TestIssue823(unittest.TestCase):

    """Testing #823."""

    def setUp(self):
        self.ctx = TritonContext()
        self.ctx.setArchitecture(ARCH.X86_64)
        self.ast = self.ctx.getAstContext()

    def test_reg(self):
        self.ctx.setConcreteRegisterValue(self.ctx.registers.rax, 0x1)
        var = self.ctx.symbolizeRegister(self.ctx.registers.rax)

        self.assertEqual(self.ctx.getConcreteRegisterValue(self.ctx.registers.rax), 0x1)
        self.assertEqual(self.ctx.getSymbolicRegisterValue(self.ctx.registers.rax), 0x1)
        self.assertEqual(self.ctx.getConcreteVariableValue(var), 0x1)

        self.ctx.setConcreteVariableValue(var, 0x2)
        self.assertEqual(self.ctx.getConcreteRegisterValue(self.ctx.registers.rax), 0x2)
        self.assertEqual(self.ctx.getSymbolicRegisterValue(self.ctx.registers.rax), 0x2)
        self.assertEqual(self.ctx.getConcreteVariableValue(var), 0x2)

    def test_mem(self):
        mem = MemoryAccess(0x100, 4)
        self.ctx.setConcreteMemoryValue(mem, 0x11223344)
        var = self.ctx.symbolizeMemory(mem)

        self.assertEqual(self.ctx.getConcreteMemoryValue(mem), 0x11223344)
        self.assertEqual(self.ctx.getSymbolicMemoryValue(mem), 0x11223344)
        self.assertEqual(self.ctx.getConcreteVariableValue(var), 0x11223344)

        self.ctx.setConcreteVariableValue(var, 0x55667788)
        self.assertEqual(self.ctx.getConcreteMemoryValue(mem), 0x55667788)
        self.assertEqual(self.ctx.getSymbolicMemoryValue(mem), 0x55667788)
        self.assertEqual(self.ctx.getConcreteVariableValue(var), 0x55667788)


class TestIssue825(unittest.TestCase):
    """Testing #825."""

    def setUp(self):
        self.ctx = TritonContext()
        self.ctx.setArchitecture(ARCH.X86_64)

    def test_issue(self):
        # taint eax
        self.ctx.setTaintRegister(self.ctx.registers.eax, True)
        # xor eax, eax
        inst = Instruction(b"\x31\xc0")
        self.ctx.processing(inst)
        # after execution eax should not longer be tainted
        self.assertFalse(self.ctx.isRegisterTainted(self.ctx.registers.eax))


class TestIssue860(unittest.TestCase):
    """Testing #860."""

    def setUp(self):
        self.ctx = TritonContext(ARCH.X86_64)
        self.ast = self.ctx.getAstContext()

    def test_issue(self):
        x = self.ast.variable(self.ctx.newSymbolicVariable(32))
        c = self.ast.variable(self.ctx.newSymbolicVariable(32))

        x.getSymbolicVariable().setAlias('x')
        c.getSymbolicVariable().setAlias('C')

        # ((x << 8) >> 16) << 8 == x & 0xffff00
        m = self.ctx.getModel(self.ast.forall([x], ((x << 8) >> 16) << 8 == x & c))
        self.assertEqual(m[1].getValue(), 0x00ffff00)


class TestIssue862(unittest.TestCase):
    """Testing #862."""

    def setUp(self):
        self.ctx = TritonContext(ARCH.X86_64)
        self.ast = self.ctx.getAstContext()

    def test_issue(self):
        v = self.ctx.symbolizeRegister(self.ctx.registers.zf)
        self.assertEqual(v.getName(), "SymVar_0")


class TestIssue924(unittest.TestCase):
    """Testing #924."""

    def setUp(self):
        self.ctx = TritonContext(ARCH.X86_64)

    def test_issue1(self):
        ctx = TritonContext(ARCH.X86_64)
        ctx.setConcreteRegisterValue(ctx.registers.rip, 0x4006e7)
        inst = Instruction(b"\xeb\x02")
        ctx.processing(inst)
        self.assertEqual(ctx.getConcreteRegisterValue(ctx.registers.rip), 0x4006eb)

    def test_issue2(self):
        ctx = TritonContext(ARCH.X86_64)
        inst = Instruction(b"\xeb\x02")
        inst.setAddress(0x4006e7)
        ctx.processing(inst)
        self.assertEqual(ctx.getConcreteRegisterValue(ctx.registers.rip), 0x4006eb)


class TestIssue945(unittest.TestCase):
    """Testing #945."""

    def setUp(self):
        self.ctx = TritonContext(ARCH.ARM32)
        self.inst1 = Instruction(b"\x70\x00\xbd\xe8") # pop {r4, r5, r6}
        self.inst2 = Instruction(b"\x70\x80\xbd\xe8") # pop {r4, r5, r6, pc}

    def test_issue(self):
        self.ctx.processing(self.inst1)
        self.ctx.processing(self.inst2)

        self.assertEqual(self.inst1.isControlFlow(), False)
        self.assertEqual(self.inst2.isControlFlow(), True)
