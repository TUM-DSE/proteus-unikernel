## Usage

```
bash run_benchmark.sh <evaluation type> <iteration>
```

---
**Eviction:** measure the overheads to evict/resume FPGA states
```
bash run_benchmark.sh fpga_state_oh 10
```
**Migration:** measure the overheads to save/restore VM snapshots to/from memory
```
bash run_benchmark.sh migration_oh 10
```

**Checkpoint:** measure the overheads to save/restore VM snapshots to/from disk
```
bash run_benchmark.sh vm_state_oh 10
```

**Synchronization:** measure FPGA sync time when our optimization is applied (split big requests into smaller ones)
```
bash run_benchmark.sh sync_oh 10
```

