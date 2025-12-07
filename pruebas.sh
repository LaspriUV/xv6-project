#!/usr/bin/env bash

# Número de ejecuciones para promediar
RUNS=5

AVG_TAT_SUM=0
AVG_WT_SUM=0

for i in $(seq 1 $RUNS); do
  echo "=== Run $i ==="

  # Ejecutar xv6 en modo nox y alimentar el comando bench desde stdin
  # -nographic / -display none depende de tu Makefile / QEMUOPTS.
  make qemu-nox < <(printf "bench\n") 2>/dev/null | tee run_$i.log

  # Extraer la línea que imprime tu programa (ajusta el patrón)
  line=$(grep "AVG_TAT" run_$i.log | tail -n 1)

  # Suponiendo formato: "AVG_TAT=<tat> AVG_WT=<wt>"
  tat=$(echo "$line" | sed -n 's/.*AVG_TAT=\([0-9]*\).*/\1/p')
  wt=$(echo "$line" | sed -n 's/.*AVG_WT=\([0-9]*\).*/\1/p')

  AVG_TAT_SUM=$((AVG_TAT_SUM + tat))
  AVG_WT_SUM=$((AVG_WT_SUM + wt))
done

echo "=================================="
echo "Runs: $RUNS"
echo "Mean AVG_TAT=$((AVG_TAT_SUM / RUNS))"
echo "Mean AVG_WT=$((AVG_WT_SUM / RUNS))"
