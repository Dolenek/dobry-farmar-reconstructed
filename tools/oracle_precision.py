"""Run independent original-method comparisons in process/device x87 contexts."""
import subprocess
def run_precision_cases(executable,arguments,device_only=False,timeout=60):
    results=[]
    contexts=('x87-pc24',) if device_only else ('process-default','x87-pc24')
    for precision in contexts:
        command=[str(executable),*(str(value) for value in arguments)]
        if precision=='x87-pc24': command.append('pc24')
        result=subprocess.run(command,capture_output=True,text=True,timeout=timeout)
        if result.returncode:
            raise RuntimeError(f'{executable.name} failed ({result.returncode}, {precision}): {result.stderr}')
        results.append((precision,result.stdout))
    return results
