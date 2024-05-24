# Sistemi Operativi



## Homework

Contiene una serie di cartelle dei vari codici proposti a lezione durante l'anno. Sono stati commentati per rendere la compresione e il ripasso per il futuro me ancora più comprensibile :

- Homework1 : Realizzazione dei file CMake con annesso anche un esercizio

- Homework2 : Una serie di piccoli esercizi con anche una prima implementazione delle liste polimorfiche

- Homework3 : Presentazione di come di deve usare l' ucontext e di un esempio di context switch

- Homework4 : Studio dell'implementazione del memory allocator 

- Homework5 : Codice relativo a DisastrOS

- Homework6 : Implementazione di un CPU scheduler con annesso anche degli esercizi di esame svolti 

- Homework7 : Codice relativo alla gestione della memoria, contiene una serie di implementazioni fatte durante il corso


## Progetto : Misuratore di corrente Arduino

### Obiettivo

 Misurare la corrente campionando un canale CA al quale è collegato il misuratore.

### Features

   Vengono conservate:
   - [ ] Le statistiche dell'ultima ora
   - [ ] Le statistiche dell'ultimo giorno
   - [ ] Le statistiche dei giorni dell'ultimo mese 
   - [ ] Le statistiche dei mesi dell'ultimo anno 

   Il sistema implementa una semplice interfaccia che permetta di
   - [ ] impostare la modalità "on-line", quando scrive un campione ogni x secondi sulla seriale
   - [ ] cancellare le statistiche
   - [ ] interrogare le statistiche

