# gnuplot offline_output.plt

# Imposta il terminale di output
set terminal pngcairo enhanced font "arial,10"

# Imposta le configurazioni generali del grafico
set xlabel "Index"
set ylabel "Voltage"
set yrange [-0.5:5.5]  # Imposta il range dell'asse y

# Loop per ogni dataset
do for [i=1:7] {

    set output sprintf("output_datasetfile%d.png", i)  # Specifica il nome del file di output
    set title sprintf("Channel %d", i)

    plot sprintf("../datafile%d.dat", i)using 0:1 with lines lc rgb "red" title sprintf("Channel %d", i)
}
