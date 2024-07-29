# gnuplot -persist streaming_output.plt

# Imposta il terminale di output
set terminal pngcairo enhanced font "arial,10"

# Imposta le configurazioni generali del grafico
set xlabel "Index"
set ylabel "Voltage"
set yrange [-0.5:5.5]  # Imposta il range dell'asse y

# Numero di punti da visualizzare
num_points = 100

# Loop infinito per aggiornare i grafici
while (1) {
    do for [i=1:8] {
        set output sprintf("output_datasetfile%d.png", i)  # Specifica il nome del file di output
        set title sprintf("Channel %d", i)
        
        # Controlla se il file di dati contiene dati validi
        system(sprintf("tail -n %d ../datafile%d.dat > temp.dat", num_points, i))
        stats "temp.dat" using 1 nooutput
        if (STATS_records > 0) {
            plot "temp.dat" using 0:1 with lines lc rgb "red" title sprintf("Channel %d", i)
        } else {
            print sprintf("Skipping data file %d with no valid points", i)
        }
        
        # Elimina il file temporaneo dopo l'uso
        system("rm temp.dat")
    }
    pause 1  # Pausa di 1 secondo prima di aggiornare di nuovo i grafici
}