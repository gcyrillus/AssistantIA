# Manuale di Utilizzo : Assistente IA
## Versione Alpha 0.0.0.1

Il plugin **Assistente IA** trasforma Notepad++ in un ambiente di sviluppo assistito dall'intelligenza artificiale. Ti consente di interagire con diversi modelli di linguaggio direttamente dal tuo editor per analizzare, correggere o generare codice.
Il plugin è proposto con, al minimo, 4 assistenti (API): groq, openrouter, anthropic e gemini.

___

## Installazione

### Prerequisiti
Versione (c) Windows 64 bit e Notepad++ x64: **almeno Windows 10** (versione **1809** o successiva) o Windows 11**(*)**.
**(*)** Python: le versioni **3.13** e **3.14**, il supporto per **Windows 7, 8** e **8.1** è stato definitivamente abbandonato.
Questo plugin è compilato con l'opzione `/MD` ed è esclusivamente progettato per le architetture **x64**. 
- **Verificare la compatibilità**: In Notepad++, vai nel menu `?` (Aiuto) > `Informazioni di debug...`. Verifica che la riga dell'architettura menzioni bene `64-bit x64`.
- **Dipendenza di sistema**: Essendo compilato in `/MD`, il pacchetto ***Microsoft Visual C++ Redistributable*** deve essere installato sulla tua macchina affinché Windows possa caricare il plugin.

### Download e installazione
1. Scarica l'ultima versione del plugin da GitHub. https://github.com/gcyrillus/AssistantIA
2. Estraisci l'archivio scaricato in una directory temporanea a tua scelta.
3. Copia la cartella completa `AssistenteIA` (che contiene la DLL e le sue dipendenze).
4. Incolla questa cartella nella directory dei plugin di Notepad++ (solitamente situata in `C:\Program Files\Notepad++\plugins`).
5. Riavvia Notepad++.

___

## Utilizzo

### Primo avvio
Quando avvii Notepad++ per la prima volta dopo l'installazione, il plugin rileverà che non ha ancora impostazioni. Una finestra di dialogo si aprirà automaticamente per guidarti nella configurazione iniziale delle tue chiavi API.
Per default, il plugin è in francese, se rileva un'altra lingua di configurazione di Notepad++ tradurrà i file di lingua e di aiuto nella lingua corrispondente a quella di Notepad++. 

### Configurazione
Accessibile in qualsiasi momento tramite la pagina di configurazione, puoi gestire le tue impostazioni:
- **Assistente predefinito**: Il plugin utilizza **Groq** per default per risposte quasi istantanee.
- **Chiavi API**: Devi inserire la tua chiave per comunicare con l'IA. Per Groq, puoi ottenere una chiave gratuitamente sulla console dello sviluppatore (*console.groq.com*).
- **Modelli**: L'interfaccia ti consente di aggiungere, modificare o aggiornare i modelli di linguaggio disponibili per ogni fornitore.
- **Registrazione dei log**: Una casella di controllo ti consente di attivare la salvataggio della cronologia delle tue interazioni con l'IA direttamente nei file di configurazione.

___

## Utilizzo quotidiano

### Il Pannello di Chat
È il tuo centro di comando principale per discutere con l'IA:
- **Scorciatoia di apertura**: `ALT + I`.
- **Modello attivo**: Utilizza la lista a discesa in alto del pannello per passare rapidamente tra i diversi modelli proposti dal tuo fornitore.
- **Input**: Digita le tue istruzioni nella zona di testo situata in basso.
- **Invio**: Premi il pulsante *Invia* o utilizza la tua scorciatoia da tastiera abituale per inviare la richiesta.
- **Cronologia**: La finestra centrale visualizza le tue interazioni in tempo reale. Se l'opzione è selezionata nella configurazione, questi log saranno conservati.

### Menu Contestuale
Il menu contestuale (clic destro) ti consente di interagire rapidamente con il tuo **codice selezionato**:
- **Codice selezionato**: È limitato ai primi 20.000 caratteri. I fornitori (API) inclusi sono versioni gratuite. Notepad++ ti indica nella barra di stato la lunghezza della stringa selezionata e il numero di righe: ad esempio *Sel: 787: 7*.
- **Scorciatoia di apertura**: `ALT + Q`.
- **Porre una domanda all'IA**: Apre il pannello per porre la tua domanda (richiesta).
- **Conferma**: Invia la tua richiesta all'IA iniettando direttamente il contesto della tua selezione attuale in background.
- **Risposta dell'IA**: La risposta dell'IA viene visualizzata in splitview in un documento provvisorio *AssistenteIA.txt* di fronte al documento della tua selezione.

___


## La console:

Il plugin include una console. Questa può essere visualizzata o meno e può essere staccata in una finestra flottante. Visualizza informazioni relative alle azioni in corso e agli eventuali errori. Dispone anche di un prompt Python.
È utile a scopo di debug principale.
___

## Funzionalità Avanzate: Script e Template

Il plugin è progettato per essere estensibile dagli utenti avanzati che desiderano personalizzare la propria esperienza e aggiungere nuovi fornitori (API).

- **Creazione di nuovi fornitori**: Tramite il menu del plugin, seleziona `Nuovo script` per integrare una nuova API IA.
- **Struttura di uno script**: Ogni nuovo script è costruito su una base comune generata automaticamente. Non ti resta che finalizzare la logica di richiesta propria del fornitore.
- **Modifica dei prompt**: Puoi modificare il *System Prompt* direttamente nei file Python di ogni fornitore per adattare il comportamento dell'IA alle tue esigenze modificando il valore tradotto di: `system_prompt = _T("MESSAGES", "MSG_PROV_SYS_PROMPT_CODE")`. ad esempio *`system_prompt ="Sei un esperto in PHP"`, *"Rispondi solo in francese, ..."`*, ecc.

___

## Risoluzione dei problemi (Troubleshooting)

Se il plugin non si carica o visualizza un errore, verifica i punti seguenti:

- **Assenza di risposta**: Verifica prima di tutto che sei connesso a Internet.
- **Elenco dei modelli vuoto**: Verifica la tua connessione internet, assicurati che la tua chiave API sia valida e verifica che il nome esatto del modello sia riconosciuto dal fornitore.
- **Crash all'avvio o Plugin invisibile**: Assicurati che il *Visual C++ Redistributable (x64)* sia installato sulla tua macchina. Questa installazione è strettamente richiesta per il funzionamento della DLL e il caricamento delle librerie Python associate (`.pyd`).

___

*Nota: Questo plugin è uno strumento di assistenza alla codifica. Rimani critico. Assicurati di rileggere e testare sempre il codice generato prima di integrarlo nei tuoi progetti di produzione.*
