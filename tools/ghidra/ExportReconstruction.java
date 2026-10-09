// Export evidence for reconstruction. Pseudocode is explicitly not compilable source.
//@category Reconstruction

import java.io.*;
import java.nio.charset.StandardCharsets;
import java.nio.file.*;
import java.util.*;
import com.google.gson.Gson;
import com.google.gson.GsonBuilder;
import ghidra.app.decompiler.*;
import ghidra.app.script.GhidraScript;
import ghidra.program.model.address.Address;
import ghidra.program.model.listing.*;
import ghidra.program.model.symbol.*;

public class ExportReconstruction extends GhidraScript {
    private final Gson gson = new GsonBuilder().setPrettyPrinting().create();

    private void json(Path file, Object value) throws IOException {
        try (Writer out = Files.newBufferedWriter(file, StandardCharsets.UTF_8)) {
            gson.toJson(value, out);
        }
    }

    @Override
    public void run() throws Exception {
        String[] args = getScriptArgs();
        if (args.length < 1) throw new IllegalArgumentException("Output directory required");
        Path root = Paths.get(args[0]);
        Files.createDirectories(root);
        int timeout = args.length > 1 ? Integer.parseInt(args[1]) : 30;
        Map<String, Object> summary = new LinkedHashMap<>();
        summary.put("program", currentProgram.getName());
        summary.put("language", currentProgram.getLanguageID().toString());
        summary.put("compiler_spec", currentProgram.getCompilerSpec().getCompilerSpecID().toString());
        summary.put("image_base", currentProgram.getImageBase().toString());
        summary.put("input_sha256", currentProgram.getExecutableSHA256());
        summary.put("status", "export_in_progress");
        json(root.resolve("summary.json"), summary);

        List<Map<String, Object>> symbols = new ArrayList<>();
        SymbolIterator si = currentProgram.getSymbolTable().getAllSymbols(true);
        while (si.hasNext()) {
            monitor.checkCancelled();
            Symbol s = si.next();
            Map<String, Object> row = new LinkedHashMap<>();
            row.put("address", s.getAddress().toString());
            row.put("name", s.getName(true));
            row.put("type", s.getSymbolType().toString());
            row.put("source", s.getSource().toString());
            row.put("external", s.isExternal());
            symbols.add(row);
        }
        json(root.resolve("symbols.json"), symbols);

        List<Map<String, Object>> strings = new ArrayList<>();
        DataIterator di = currentProgram.getListing().getDefinedData(true);
        while (di.hasNext()) {
            monitor.checkCancelled();
            Data d = di.next();
            if (!(d.getValue() instanceof String)) continue;
            Map<String, Object> row = new LinkedHashMap<>();
            row.put("address", d.getAddress().toString());
            row.put("text", d.getValue().toString());
            List<String> refs = new ArrayList<>();
            for (Reference ref : currentProgram.getReferenceManager().getReferencesTo(d.getAddress()))
                refs.add(ref.getFromAddress().toString());
            row.put("references", refs);
            strings.add(row);
        }
        json(root.resolve("strings.json"), strings);

        List<Map<String, Object>> types = new ArrayList<>();
        Iterator<ghidra.program.model.data.DataType> ti =
            currentProgram.getDataTypeManager().getAllDataTypes();
        while (ti.hasNext()) {
            monitor.checkCancelled();
            ghidra.program.model.data.DataType t = ti.next();
            Map<String, Object> row = new LinkedHashMap<>();
            row.put("path", t.getPathName());
            row.put("length", t.getLength());
            row.put("description", t.toString());
            types.add(row);
        }
        json(root.resolve("types.json"), types);

        DecompInterface decompiler = new DecompInterface();
        DecompileOptions options = new DecompileOptions();
        options.grabFromProgram(currentProgram);
        decompiler.setOptions(options);
        decompiler.toggleCCode(true);
        decompiler.toggleSyntaxTree(false);
        if (!decompiler.openProgram(currentProgram)) {
            decompiler.dispose();
            throw new IOException("Cannot open decompiler: " + decompiler.getLastMessage());
        }
        int count = 0, success = 0, failed = 0;
        try (BufferedWriter code = Files.newBufferedWriter(root.resolve("pseudocode.c"), StandardCharsets.UTF_8);
             BufferedWriter index = Files.newBufferedWriter(root.resolve("functions.jsonl"), StandardCharsets.UTF_8)) {
            code.write("/* GHIDRA PSEUDOCODE: NOT A COMPILABLE OR VERIFIED RECONSTRUCTION. */\n");
            FunctionIterator fi = currentProgram.getFunctionManager().getFunctions(true);
            while (fi.hasNext()) {
                monitor.checkCancelled();
                Function f = fi.next();
                Map<String, Object> row = new LinkedHashMap<>();
                row.put("address", f.getEntryPoint().toString());
                row.put("name", f.getName(true));
                row.put("signature", f.getSignature().toString());
                row.put("body_bytes", f.getBody().getNumAddresses());
                row.put("body_min", f.getBody().getMinAddress().toString());
                row.put("body_max", f.getBody().getMaxAddress().toString());
                row.put("no_return", f.hasNoReturn());
                row.put("thunk", f.isThunk());
                List<String> callees = new ArrayList<>();
                for (Function c : f.getCalledFunctions(monitor)) callees.add(c.getEntryPoint().toString());
                Collections.sort(callees);
                row.put("callees", callees);
                if (!f.isThunk() && !f.isExternal()) {
                    DecompileResults result = decompiler.decompileFunction(f, timeout, monitor);
                    if (result.decompileCompleted() && result.getDecompiledFunction() != null) {
                        code.write("\n/* ADDRESS " + f.getEntryPoint() + " NAME " + f.getName(true).replace("*/", "") + " */\n");
                        code.write(result.getDecompiledFunction().getC());
                        code.write("\n");
                        row.put("decompiled", true);
                        success++;
                    } else {
                        row.put("decompiled", false);
                        row.put("error", result.getErrorMessage());
                        failed++;
                    }
                } else row.put("decompiled", false);
                // Compact JSON lines are independently readable after an interrupted export.
                index.write(new Gson().toJson(row));
                index.newLine();
                count++;
                if (count % 100 == 0) {
                    code.flush(); index.flush();
                    println("EXPORT " + count + " functions, " + success + " decompiled, " + failed + " errors");
                }
            }
        } finally {
            decompiler.dispose();
        }
        summary.put("status", "export_complete");
        summary.put("function_count", count);
        summary.put("decompiled_count", success);
        summary.put("failed_count", failed);
        summary.put("string_count", strings.size());
        summary.put("symbol_count", symbols.size());
        summary.put("type_count", types.size());
        json(root.resolve("summary.json"), summary);
        println("EXPORT COMPLETE " + currentProgram.getName() + ": " + success + "/" + count);
    }
}
