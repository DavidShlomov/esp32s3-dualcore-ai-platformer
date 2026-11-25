import subprocess


def run_seed(seed):
    cmd = ["./bench", str(seed)]
    result = subprocess.run(cmd, text=True, capture_output=True)
    result_lines = result.stdout.splitlines()

    for line in result_lines:
        if line.startswith("RESULT"):
            words = line.split()
            fields = {}
            for word in words[1:]:
                key, val = word.split("=")

                if "." in val:
                    num_val = float(val)
                else:
                    num_val = int(val)

                fields[key] = num_val
            return fields


runs = []
for i in range(1, 21):
    fields = run_seed(i)

    if fields is None:
        print(f"No fields in seed {i}")
    else:
        print(f"seed: {fields['seed']}\tfitness: {fields['fitness']}\tDead: {fields['dead']}")
        runs.append(fields)

num_of_seeds = len(runs)
num_of_failed = 0
num_of_map_ok = 0
total_run_time = 0

for run in runs:
    if run["dead"] == 1:
        num_of_failed += 1
    if run["map_ok"] == 1:
        num_of_map_ok += 1
    total_run_time += run["train_ms"]

try:
    average_train_ms = total_run_time / num_of_seeds
    fail_percentage = num_of_failed / num_of_seeds
except ZeroDivisionError:
    print("Error, no seeds ran.")
else:
    print(f"num_of_seeds: {num_of_seeds}\tnum_of_failed: {num_of_failed}\tnum_of_map_ok: {num_of_map_ok}\tfail_percentage: {fail_percentage*100:.2f}%\taverage_train_ms: {average_train_ms:.2f}ms")
