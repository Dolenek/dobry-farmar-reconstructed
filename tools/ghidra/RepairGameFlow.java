// Repair a verified bad no-return annotation in the supplied game working copy.
//@category Reconstruction
import java.nio.charset.StandardCharsets;
import java.nio.file.*;
import java.util.*;
import com.google.gson.GsonBuilder;
import ghidra.app.cmd.function.CreateFunctionCmd;
import ghidra.app.script.GhidraScript;
import ghidra.program.model.address.Address;
import ghidra.program.model.listing.*;
import ghidra.program.model.symbol.Reference;

public class RepairGameFlow extends GhidraScript {
    @Override
    public void run() throws Exception {
        if (!"9815152da86429e1fb5f818ea8465229a0e66f50776e7e66c08d60049e41984a".equalsIgnoreCase(currentProgram.getExecutableSHA256()))
            throw new IllegalArgumentException("Repair is specific to the inventoried game binary");
        Address address = toAddr(0x005809d0L);
        Function probe = currentProgram.getFunctionManager().getFunctionAt(address);
        if (probe == null || !"__chkstk".equals(probe.getName()))
            throw new IllegalArgumentException("Expected identified __chkstk function");
        if (getByte(toAddr(0x005809e4L)) != (byte)0xc3 || getByte(toAddr(0x00580a0cL)) != (byte)0xc3)
            throw new IllegalArgumentException("Expected RET instructions in __chkstk");
        Map<String, Object> report = new LinkedHashMap<>();
        report.put("input_sha256", currentProgram.getExecutableSHA256());
        report.put("probe_address", address.toString());
        report.put("previous_no_return", probe.hasNoReturn());
        // Both free thunks delegate to a CRT routine whose epilogue returns.
        long[] freeAddresses = {0x005803afL, 0x00580b56L, 0x00580b5bL};
        if (getByte(toAddr(0x00580bcbL)) != (byte)0xc3)
            throw new IllegalArgumentException("Expected RET in CRT free epilogue");
        List<Function> targets = new ArrayList<>();
        targets.add(probe);
        List<Map<String, Object>> annotations = new ArrayList<>();
        for (long freeAddress : freeAddresses) {
            Function free = currentProgram.getFunctionManager().getFunctionAt(toAddr(freeAddress));
            if (free == null || !"_free".equals(free.getName()))
                throw new IllegalArgumentException("Expected identified CRT free routine");
            targets.add(free);
        }
        // Dynamic ESP manipulation confuses discovery; preserve the verified returning annotation.
        setAnalysisOption(currentProgram, "Non-Returning Functions - Discovered", "false");
        for (Function target : targets) {
            Map<String, Object> annotation = new LinkedHashMap<>();
            annotation.put("address", target.getEntryPoint().toString());
            annotation.put("name", target.getName());
            annotation.put("previous_no_return", target.hasNoReturn());
            annotations.add(annotation);
            target.setNoReturn(false);
        }
        report.put("annotations", annotations);
        List<Function> callers = new ArrayList<>();
        List<Long> before = new ArrayList<>();
        List<Address> continuations = new ArrayList<>();
        for (Function target : targets) {
        for (Reference ref : currentProgram.getReferenceManager().getReferencesTo(target.getEntryPoint())) {
            monitor.checkCancelled();
            Instruction instruction = getInstructionAt(ref.getFromAddress());
            if (instruction == null || !instruction.getFlowType().isCall()) continue;
            Function caller = getFunctionContaining(instruction.getAddress());
            if (caller != null && !callers.contains(caller)) {
                callers.add(caller);
                before.add(caller.getBody().getNumAddresses());
            }
            instruction.setFlowOverride(FlowOverride.NONE);
            Address continuation = instruction.getMaxAddress().add(1);
            instruction.setFallThrough(continuation);
            continuations.add(continuation);
        }
        }
        for (Address continuation : continuations) {
            monitor.checkCancelled();
            disassemble(continuation);
        }
        List<Map<String, Object>> changes = new ArrayList<>();
        for (int i = 0; i < callers.size(); i++) {
            Function caller = callers.get(i);
            CreateFunctionCmd.fixupFunctionBody(currentProgram, caller, monitor);
            Map<String, Object> row = new LinkedHashMap<>();
            row.put("address", caller.getEntryPoint().toString());
            row.put("before_bytes", before.get(i));
            row.put("after_bytes", caller.getBody().getNumAddresses());
            changes.add(row);
        }
        report.put("repaired_callers", changes);
        String[] args = getScriptArgs();
        if (args.length > 0) {
            Path output = Paths.get(args[0]);
            Files.createDirectories(output);
            Files.writeString(output.resolve("flow-repair.json"),
                new GsonBuilder().setPrettyPrinting().create().toJson(report), StandardCharsets.UTF_8);
        }
        println("Verified __chkstk return; repaired " + changes.size() + " caller bodies");
    }
}
