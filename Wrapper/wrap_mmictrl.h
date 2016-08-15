#ifndef WRAP_CNC_H_INCLUDED
#define WRAP_CNC_H_INCLUDED

#include <memory>

namespace wrap
{

struct transfer_type
{
	// Warten auf Abschluss der Dateiübertragung
	std::int32_t wait();
};

struct expression_error_info
{
	std::string const &expression; // Der fehlerhafte Ausdruck
	std::size_t index; // Index der fehlerhaften Stelle
};

struct can_transfer_state_tr_type
{
	std::string function_name; // Name der aktuell ausgeführten Funktion
	std::uint32_t number_of_objects; // Anzahl der zu übertragenden Objekten
	std::uint32_t size_of_objects; // Gesamtgröße aller zu übertragenden Objekten in Bytes
	std::uint32_t transferred_objects; // Anzahl der bereits übertragenden Objekten
	std::uint32_t transferred_size; // Größe der bereits übertragenden Objekten in Bytes
	std::shared_point<can_object_desc_tr_type> act_object; // aktuelles Objekt
};

struct can_object_desc_tr_type
{
	std::shared_pointer<can_object_desc_tr_type> next; // Pointer to next description
	std::uint16_t obj_idx; // CANopen object index
	std::uint8_t sub_idx; // CANopen sub index
	std::uint8_t node_id; // CANopen node ID
	std::int32_t max_size; // maximum size of the objects in bytes
	std::vector<std::uint8_t> data; // buffer for storing the data bytes in object
	std::int32_t result; // bytes transferred or error code if < 0
	std::uint32_t abort_code; // SDO abort code if available
};

typedef std::shared_poiner<handle_type> context_pointer_type;

enum class callback_type_type
{
	/*
	 * Statusmeldungen für Firmwareübertragung zur Steuerung.
	 * ulParam enthält den Transfer-Handle der entsprechenden Übertragung.
	 * ncrGetTransferState und ncrGetTransferType können im Callback verwendet werden,
	 * um Übertragungsstatus abzufragen.
	 */
	VK_MMI_DOWNLOAD_STATE,
	VK_MMI_DOWNLOAD_PART,
	VK_MMI_DOWNLOAD_COMPLETE,
	VK_MMI_DOWNLOAD_ERROR,
	/*
	 * Statusmeldungen einer laufenden Dateiübertragung zwischen PC und Steuerung.
	 * ulParam enthält den Transfer-Handle der entsprechenden Übertragung.
	 * ncrGetTransferState und ncrGetTransferType können im Callback verwendet werden,
	 * um Übertragungsstatus abzufragen.
	 */
	VK_MMI_TRANSFER_STATE,
	VK_MMI_TRANSFER_OK,
	VK_MMI_TRANSFER_ERROR,
	VK_MMI_TRANSFER_BREAK,
	/*
	 * Typen für ncrPostMessage
	 * Wenn iRespKey bei ncrPostMessage = -1, dann wird _SENT im Callback geschickt.
	 * Bei einem Fehler in der Übertragung zur Steuerung, wird _NOT_SENT im Callback
	 * geschickt und ulParam = iRespKey.
	 */
	VK_MMI_NCMSG_SENT,
	VK_MMI_NCMSG_NOT_SENT,
	/*
	 * Empfang einer Nachricht der Steuerung, die nicht von der DLL ausgewertet werden kann.
	 * ulParam = Pointer auf MSG_TR-Struktur (Speicher nur innerhalb des Callbacks gültig).
	 */
	VK_MMI_NCMSG_RECEIVED,
	/*
	 * Übergabe von Fehlermeldungen von Steuerung, Gateway oder DLL.
	 * ulParam = Pointer auf MSG_TR-Struktur (Speicher nur innerhalb des Callbacks gültig).
	 * Fehlermeldungen der Klasse 3 und 4 müssen quittiert werden (Senden von
	 * SB1_FEHLERQUITTING_KUC oder SB1_FEHLERQUITTING_ALLE_KUC an die Steuerung).
	 */
	VK_MMI_ERROR_MSG,
	/*
	 * Zyklischer Aufruf, bei Verwendung von blockierenden Aufrufen (alle 100ms).
	 */
	VK_MMI_CYCLIC_CALL,
	/*
	 * Meldet den Status in ulParam, wenn die Standard-Applikation vom MMI-Gateway
	 * umgeschaltet wird.
	 */
	VK_MMI_DEFAPP_STATE,
	/*
	 * Meldet den Status in ulParam (Pointer auf CAN_TRANSFER_STATE_TR) der laufenden
	 * CANopen SDO-Übertragung.
	 */
	VK_MMI_CAN_TRANSFER_STATE,
	VK_MMI_CAN_TRANSFER_COMPLETE,
};

enum class transfer_status_type
{
	TRANSFER_OK, // Transfer erfolgreich
	TRANSFER_BREAK, // Transfer abgebrochen
	TRANSFER_ERROR, // Zugriffsfehler, Prüfen der WinAPI Codes
	TRANSFER_TIMEOUT, // Transfer-Timeout
	TRANSFER_NOINIT, // Firmware-Download wurde nicht durchgeführt
	TRANSFER_QFULL // Zu viele Transferaufträge gleichzeitig aktiv
};

typedef std::function<
	callback_type_type // Aufruf-Typ
	unsigned long, // ulParam = Zusatzparameter je nach Wert von ulType
	context_pointer_type // der bei ncrConnect übergebene Kontext
> callback_function_type;

struct control
{
	// Öffnen der Steuerung mit einem Namen
	// Parameter: Name der Steuerung, Callback-Funktion
	control(std::string const &name, callback_function_type const &callback);

	// Öffnen der Standard-Steuerung
	// Parameter: Callback-Funktion
	control(callback_function_type const &callback);

	// Zeiger auf Dual-Port-RAM der Steuerung
	void *get_dpr_address();

	// Ermitteln, ob Steuerung die Firmware schon erhalten hat
	bool get_init_state();

	// Asynchrone Funktion, um die Firmware und das SPS-Programm in die Steuerung zu laden
	// Parameter: Name der Konfigurationsdatei für Firmware-Download
	transfer_type load_firmware(std::string const &config_name);

	// Synchrone Funktion, um die Firmware und das SPS-Programm in die Steuerung zu laden
	// Parameter: Name der Konfigurationsdatei für Firmware-Download
	std::int32_t load_firmware_blocked(std::string const &config_name);

	// Verbindungsaufbau zu einer bereits geladenen Steuerung für passive Applikationen, wie
	// Busserver und CodeSys.
	void connect();

	// Asynchrone Funktion, um Dateien, z.B. Maschinenkonstanten oder DIN-Programme in die
	// Steuerung zu übertragen.
	// Parameter: Dateiname, Header-String für Programmnummern bei DIN-Programmen, Steuerblock 1 für
	// Blockübertragungen
	transfer_type send_file(std::string const &name, std::string const &header, std::int32_t block1);

	// Erweiterte asynchrone Funktion, um vorwiegend Online-Programme, ab einem bestimmten Offset
	// in die Steuerung zu übertragen.
	// Parameter: Dateiname, Offset, Header-String, Steuerblock 1
	transfer_type send_file_ex(std::string const &name, std::uint64_t offset, std::string const &header,
	                           std::int32_t block1);

	// Synchrone Funktion, um Dateien, z.B. Maschinenkonstanten oder DIN-Programme in die
	// Steuerung zu übertragen.
	// Parameter: Dateiname, Header-String für Programmnummern bei DIN-Programmen, Steuerblock 1 für
	// Blockübertragungen
	transfer_status_type send_file_blocked(std::string const &name, std::string const &header,
	                                       std::int32_t block1);

	// Asynchrone Funktion, um Dateien, z.B. Maschinenkonstanten oder DIN-Programme von der
	// Steuerung zu übertragen.
	// Parameter: Dateiname, Steuerblock 1 für Blockübertragungen
	transfer_type receive_file(std::string const &name, std::int32_t block1);

	// Synchrone Funktion, um Dateien, z.B. Maschinenkonstanten oder DIN-Programme von der
	// Steuerung zu übertragen.
	// Parameter: Dateiname, Steuerblock 1 für Blockübertragungen
	transfer_status_type receive_file_blocked(std::string const &name, std::int32_t block1);

	// Lese die Werte der angegebenen P-Feldindices aus dem Parameterfeld der Steuerung.
	// Parameter: Indizes, Werte wird befüllt
	bool read_param_array(std::vector<std::int16_t> const &indices, std::vector<double> const &values);

	// Schreibe die angegebenen P-Feldwerte in das Parameterfeld der Steuerung.
	// Parameter: Indizes, Werte
	bool write_param_array(std::vector<std::int16_t> const &indices, std::vector<double> const &values);

	// Liest Echtzeit-Daten aus der Steuerung.
	// Parameter: Liste von Ausdrücken, Auswertungen werden gespeichert, Fehlerinformation wird befüllt
	bool evaluate_expression(std::vector<std::string> const &expressions, std::vector<double> &results,
	                         expression_error_info &error_info);
};

struct gateway
{
	// Zugriff auf Konfiguration des Gateways
	// Parameter: Name des Gateways (derzeit nur ein leerer String unterstützt)
	gateway(std::string const &name);

	// Anzahl der konfigurierten Verbindungen
	// Parameter: Anzahl wird befüllt
	std::int32_t get_num_of_conns(std::int32_t number);
};

}

#endif
