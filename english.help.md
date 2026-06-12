# User Manual: AI Assistant
## Version Alpha 0.0.0.1

The **AI Assistant** plugin transforms Notepad++ into an AI-assisted development environment. It allows you to interact with various language models directly from your editor to analyze, correct, or generate code.
The plugin is offered with, at a minimum, 4 assistants (APIs): groq, openrouter, anthropic, and gemini.

___

## Installation

### Prerequisites
Version (c) Windows 64 bits & Notepad++ x64: **at least Windows 10** (version **1809** or higher) or Windows 11**(*)**.
**(*)** Python: versions **3.13** and **3.14**, support for **Windows 7, 8**, and **8.1** has been permanently discontinued.
This plugin is compiled with the `/MD` option and is exclusively designed for **x64** architectures. 
- **Check compatibility**: In Notepad++, go to the `?` (Help) > `Debug Info...` menu. Check that the architecture line mentions `64-bit x64`.
- **System dependency**: Being compiled in `/MD`, the ***Microsoft Visual C++ Redistributable*** package must be installed on your machine for Windows to load the plugin.

### Download and Installation
1. Download the latest version of the plugin from GitHub. https://github.com/gcyrillus/AssistantIA
2. Unzip the downloaded archive to a temporary directory of your choice.
3. Copy the complete `AssistantIA` folder (which contains the DLL and its dependencies).
4. Paste this folder into the Notepad++ plugins directory (usually located in `C:\Program Files\Notepad++\plugins`).
5. Restart Notepad++.

___

## Usage

### First Launch
When you launch Notepad++ for the first time after installation, the plugin will detect that it does not have any settings yet. A dialog box will open automatically to guide you through the initial configuration of your API keys.
By default, the plugin is in French; if it detects a different language configuration in Notepad++, it will translate the language and help files into the corresponding language.

### Configuration
Accessible at any time via the configuration page, you can manage your settings:
- **Default Assistant**: The plugin uses **Groq** by default for almost instantaneous responses.
- **API Keys**: You must enter your key to communicate with the AI. For Groq, you can get a free key on the developer console (*console.groq.com*).
- **Models**: The interface allows you to add, modify, or update the language models available for each provider.
- **Log recording**: A checkbox allows you to activate the saving of your interaction history with the AI directly in the configuration files.

___

## Daily Use

### The Chat Panel
This is your main command center for discussing with the AI:
- **Shortcut to open**: `ALT + I`.
- **Active Model**: Use the dropdown list at the top of the panel to quickly switch between the different models offered by your provider.
- **Input**: Type your instructions in the text area at the bottom.
- **Send**: Click the *Send* button or use your usual keyboard shortcut to submit the request.
- **History**: The central window displays your exchanges in real-time. If the option is checked in the configuration, these logs will be saved.

### Context Menu
The context menu (right-click) allows you to interact quickly with your **selected code**:
- **Selected code**: It is limited to the first 20,000 characters. The included providers (APIs) are free versions. Notepad++ indicates in its status bar the length of the selected string and the number of lines: ex: *Sel: 787: 7*.
- **Shortcut to open**: `ALT + Q`.
- **Ask the AI a question**: Opens the panel to ask your question (request).
- **Validate**: Sends your request to the AI by directly injecting the context of your current selection in the background.
- **AI response**: The AI's response is displayed in a split view in a temporary document *AssistantIA.txt* facing your selection document.

___

## The Console:

The plugin includes a console. It can be displayed or not and can be detached into a floating window. It displays information about ongoing actions and potential errors. It also has a Python prompt.
It is useful for debugging purposes mainly.
___

## Advanced Features: Scripts and Templates

The plugin is designed to be extensible by advanced users who want to customize their experience and add new providers (APIs).

- **Creating new providers**: Via the plugin menu, select `New script` to integrate a new AI API.
- **Script structure**: Each new script is built on a common base generated automatically. You just need to finalize the request logic specific to the provider.
- **Modifying prompts**: You can modify the *System Prompt* directly in the Python files of each provider to adapt the AI's behavior to your needs by modifying the translated value of: `system_prompt = _T("MESSAGES", "MSG_PROV_SYS_PROMPT_CODE")`. ex: *`system_prompt ="You are a PHP expert"`, `"Respond only in French, ..."`*, etc.

___

## Troubleshooting

If the plugin does not load or displays an error, check the following points:

- **No response**: First, check that you are connected to the internet.
- **Empty model list**: Check your internet connection, ensure that your API key is valid, and verify that the exact model name is recognized by the provider.
- **Crash at startup or invisible plugin**: Ensure that the *Visual C++ Redistributable (x64)* is installed on your machine. This installation is strictly required for the plugin to work and for the Python libraries to load (`.pyd`).

___

*Note: This plugin is a coding assistance tool. Remain critical. Always review and test the generated code before integrating it into your production projects.*
