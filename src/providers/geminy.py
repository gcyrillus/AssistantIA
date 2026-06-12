import urllib.request
import json
import ssl
import os
from datetime import datetime
import config_manager
import re

HISTORIQUE_CHAT = []

def mettre_a_jour_modeles(api_key):
    # La bonne URL de compatibilité OpenAI fournie par Google
    url = "https://generativelanguage.googleapis.com/v1beta/openai/models"
    req = urllib.request.Request(url)
    req.add_header("Authorization", f"Bearer {api_key}")
    req.add_header("Content-Type", "application/json")
    req.add_header("User-Agent", "Mozilla/5.0 (Windows NT 10.0; Win64; x64) NotepadPlusPlus/AssistantIA")
    
    ctx = ssl._create_unverified_context()
    try:
        with urllib.request.urlopen(req, timeout=7, context=ctx) as response:
            donnees = json.loads(response.read().decode("utf-8"))
            dictionnaire_modeles = {}
            for m in donnees.get("data", []):
                m_id = m.get("id")
                # On filtre logiquement sur le nom "gemini"
                if "gemini" in m_id:
                    dictionnaire_modeles[m_id] = m_id
            return dictionnaire_modeles
    except Exception as e:
        msg = _T("MESSAGES", "MSG_PROV_ERR_MAJ_MODELES", "[AssistantIA][{provider}] Erreur maj modeles : {e}")
        print(msg.format(provider="Gemini", e=str(e)))
        return {"error": str(e)}

def verifier_cle(api_key):
    res = mettre_a_jour_modeles(api_key)
    if "error" in res:
        return False
    return True

def enregistrer_historique_dans_fichier(provider, modele):
    global HISTORIQUE_CHAT
    try:
        log_dir = os.path.join(config_manager.APPDATA_DIR, "Chats")
        if not os.path.exists(log_dir):
            os.makedirs(log_dir)
            
        date_str = datetime.now().strftime("%Y%m%d-%H%M")
        modele_clean = re.sub(r'[^a-zA-Z0-9_-]', '_', modele)
        nom_fichier = f"assistant-{provider.lower()}-{modele_clean}-{date_str}.md"
        chemin_complet = os.path.join(log_dir, nom_fichier)
        
        with open(chemin_complet, "w", encoding="utf-8") as f:
            date_formatee = datetime.now().strftime('%d/%m/%Y %H:%M')
            
            # Injection des traductions avec formatage
            f.write(_T("MESSAGES", "MSG_PROV_SESSION_TITRE").format(date=date_formatee))
            f.write(_T("MESSAGES", "MSG_PROV_INFO_FOURNISSEUR").format(provider=provider))
            f.write(_T("MESSAGES", "MSG_PROV_INFO_MODELE").format(modele=modele))
            
            nom_user = _T("MESSAGES", "MSG_PROV_AUTEUR_USER")
            nom_ia = _T("MESSAGES", "MSG_PROV_AUTEUR_IA")
            
            for msg in HISTORIQUE_CHAT:
                if msg["role"] == "system": continue
                auteur = nom_user if msg["role"] == "user" else nom_ia
                f.write(f"### {auteur} :\n{msg['content']}\n\n---\n\n")
                
        return chemin_complet
    except Exception as e:
        msg = _T("MESSAGES", "MSG_PROV_ERR_SAVE_MD")
        print(msg.format(provider="Groq", e=str(e)))
        return None

def envoyer_chat(prompt, stocker_log, modele_a_la_volee=None):
    global HISTORIQUE_CHAT
    
    api_key = config_manager.obtenir_api_key()
    provider = config_manager.obtenir_provider_actif()
    
    if modele_a_la_volee and modele_a_la_volee.strip():
        modele = modele_a_la_volee
    else:
        modele = config_manager.obtenir_modele_selectionne()
        
    if not api_key:
        print( _T("MESSAGES", "MSG_PROV_ERR_NO_KEY_LOG").format(provider=provider.capitalize()))
        return _T("MESSAGES", "MSG_PROV_ERR_NO_KEY_SYS")
    if not modele:
        modele = "gemini-3.5-flash"

    print(_T("MESSAGES", "MSG_PROV_ENVOI_REQ").format(provider=provider.capitalize()))
    print(_T("MESSAGES", "MSG_PROV_MODELE_ACTIF").format(modele=modele))
    print(_T("MESSAGES", "MSG_PROV_TAILLE_PROMPT").format(taille=len(prompt)))

    if not HISTORIQUE_CHAT:
        HISTORIQUE_CHAT.append({
            "role": "system", 
            "content": _T("MESSAGES", "MSG_PROV_SYS_PROMPT_CHAT")
        })
        
    HISTORIQUE_CHAT.append({"role": "user", "content": prompt})
    
    # URL de complétion Google (Compatible OpenAI)
    url = "https://generativelanguage.googleapis.com/v1beta/openai/chat/completions"
    payload = {
        "model": modele,
        "messages": HISTORIQUE_CHAT,
        "temperature": 0.3
    }
    
    req = urllib.request.Request(url)
    req.add_header("Authorization", f"Bearer {api_key}")
    req.add_header("Content-Type", "application/json")
    req.add_header("User-Agent", "Mozilla/5.0 (Windows NT 10.0; Win64; x64) NotepadPlusPlus/AssistantIA")
    
    ctx = ssl._create_unverified_context()
    
    try:
        body = json.dumps(payload).encode("utf-8")
        print(_T("MESSAGES", "MSG_PROV_CONNEXION").format(provider=provider.capitalize()))
        
        with urllib.request.urlopen(req, data=body, timeout=20, context=ctx) as response:
            status_code = response.getcode()
            print(_T("MESSAGES", "MSG_PROV_REPONSE_OK").format(status_code=status_code))
            
            donnees = json.loads(response.read().decode("utf-8"))
            reponse_ia = donnees["choices"][0]["message"]["content"]
            
            HISTORIQUE_CHAT.append({"role": "assistant", "content": reponse_ia})
            
            if stocker_log:
                chemin = enregistrer_historique_dans_fichier(provider, modele)
                if chemin:
                    print(_T("MESSAGES", "MSG_PROV_ARCHIVE_OK").format(chemin=chemin))
                
            return reponse_ia
            
    except Exception as e:
        if HISTORIQUE_CHAT and HISTORIQUE_CHAT[-1]["role"] == "user": HISTORIQUE_CHAT.pop()
        print(_T("MESSAGES", "MSG_PROV_ECHEC_REQ").format(e=str(e)))
        return _T("MESSAGES", "MSG_PROV_ERR_API").format(provider="Gemini", e=str(e))

def effacer_historique_chat():
    provider = config_manager.obtenir_provider_actif()
    global HISTORIQUE_CHAT
    HISTORIQUE_CHAT = []
    print(_T("MESSAGES", "MSG_PROV_HISTO_EFFACE").format(provider=provider.capitalize()))
    return True

def markdown_to_rtf(text_md):
    if not text_md:
        return ""
    en_tete = r"{\rtf1\ansi\ansicpg65001\uc1\deff0{\fonttbl{\f0\fnil\fcharset0 Segoe UI;}{\f1\fmodern\fcharset0 Consolas;}}{\colortbl;\red0\green0\blue0;\red40\green44\blue52;\red240\green240\blue240;}"
    text_md = text_md.replace('\r\n', '\n').replace('\r', '\n')
    lines = text_md.split('\n')
    rtf_lines = []
    dans_code = False
    for line in lines:
        if line.strip().startswith("```"):
            dans_code = not dans_code
            if dans_code:
                rtf_lines.append(r"\pard\highlight3\cf2\f1\fs19\li200 ")
            else:
                rtf_lines.append(r"\pard\highlight0\cf1\f0\fs20\li0 ")
            continue
        escaped_line = line.replace('\\', '\\\\').replace('{', '\\{').replace('}', '\\}')
        escaped_line = "".join([f"\\u{ord(c)}?" if ord(c) > 127 else c for c in escaped_line])
        if dans_code:
            rtf_lines.append(f"{escaped_line}\\par")
        else:
            if escaped_line.startswith("### "):
                escaped_line = re.sub(r'^### (.*)', r'\\pard\\b\\fs24 \1\\b0\\fs20\\par', escaped_line)
            elif escaped_line.startswith("## "):
                escaped_line = re.sub(r'^## (.*)', r'\\pard\\b\\fs28 \1\\b0\\fs20\\par', escaped_line)
            elif escaped_line.startswith("# "):
                escaped_line = re.sub(r'^# (.*)', r'\\pard\\b\\fs32 \1\\b0\\fs20\\par', escaped_line)
            elif re.match(r'^[\*\-] ', escaped_line):
                escaped_line = re.sub(r'^[\*\-] (.*)', r'\\pard\\li300\\bullet  \1\\par', escaped_line)
            elif re.match(r'^\d+\. ', escaped_line):
                escaped_line = re.sub(r'^(\d+\.) (.*)', r'\\pard\\li300 \1 \2\\par', escaped_line)
            else:
                escaped_line = r"\pard\li0 " + escaped_line + r"\par"
            escaped_line = re.sub(r'\*\*(.*?)\*\*', r'\\b \1\\b0 ', escaped_line)
            escaped_line = re.sub(r'\*(.*?)\*', r'\\i \1\\i0 ', escaped_line)
            escaped_line = re.sub(r'`(.*?)`', r'\\highlight3\\f1 \1\\highlight0\\f0 ', escaped_line)
            rtf_lines.append(escaped_line)
    return en_tete + "\n".join(rtf_lines) + "}"

def sauvegarder_log_selection_md(provider, modele, consigne, code_original, code_reponse):
    try:
        log_dir = os.path.join(config_manager.APPDATA_DIR, "bckSelect")
        if not os.path.exists(log_dir):
            os.makedirs(log_dir)
        date_file_str = datetime.now().strftime("%Y%m%d")
        modele_clean = re.sub(r'[^a-zA-Z0-9_-]', '_', modele)
        nom_fichier = f"{date_file_str}-{provider.lower()}-{modele_clean}.md"
        chemin_complet = os.path.join(log_dir, nom_fichier)
        date_entry_str = datetime.now().strftime("%Y/%m/%d %H:%M:%S")

        with open(chemin_complet, "a", encoding="utf-8") as f:
             f.write(f"## {date_entry_str}\n\n")
            f.write(f"### Question:\n{consigne}\n\n")
            f.write(f"### Selection:\n```\n{code_original}\n```\n\n")
            f.write(f"### Code IA:\n```\n{code_reponse}\n```\n\n")
            f.write("---\n\n")
            print(f"[AssistantIA][" + provider.capitalize() + "]  log sauvegarder.")
    except Exception as e:
        print(_T("MESSAGES", "MSG_PROV_ERR_SAVE_BCK").format(provider=provider.capitalize(), e=str(e)))

def demander_code_sur_selection(consigne, code_selectionne, sauvegarder_log=True):
    api_key = config_manager.obtenir_api_key()
    modele = config_manager.obtenir_modele_selectionne()
    provider = config_manager.obtenir_provider_actif()
    
    if not api_key: return _T("MESSAGES", "MSG_PROV_ERR_NO_KEY_CODE")
    if not modele:
        modele = "gemini-1.5-flash" # Correction du modèle par défaut
        
    # Correction : Suppression de la parenthèse ')' à la fin de l'URL
    url = "https://generativelanguage.googleapis.com/v1beta/openai/chat/completions"
    
    system_prompt = _T("MESSAGES", "MSG_PROV_SYS_PROMPT_CODE")
    
    payload = {
        "model": modele,
        "messages": [
            {"role": "system", "content": system_prompt},
            {"role": "user", "content": f"Consigne: {consigne}\n\nCode cible:\n{code_selectionne}"}
        ],
        "temperature": 0.2
    }
    
    req = urllib.request.Request(url)
    req.add_header("Authorization", f"Bearer {api_key}")
    req.add_header("Content-Type", "application/json")
    req.add_header("User-Agent", "Mozilla/5.0 (Windows NT 10.0; Win64; x64) NotepadPlusPlus/AssistantIA")
    
    ctx = ssl._create_unverified_context()
    
    try:
        with urllib.request.urlopen(req, data=json.dumps(payload).encode("utf-8"), timeout=20, context=ctx) as response:
            res = json.loads(response.read().decode("utf-8"))
            code = res["choices"][0]["message"]["content"].strip()
            code = re.sub(r'^```[a-zA-Z]*\n|```$', '', code, flags=re.MULTILINE)
            
            if sauvegarder_log:
                sauvegarder_log_selection_md(provider, modele, consigne, code_selectionne, code)
                
            return code
    except Exception as e:
        provider = provider.capitalize()
        return _T("MESSAGES", "MSG_PROV_ERR_REQ_CODE").format(provider="{provider.capitalize()}", e=str(e))
