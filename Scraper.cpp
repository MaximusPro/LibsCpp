#include "Scraper.h"

bool iequals(std::string_view a, std::string_view b)
{
    if (a.size() != b.size()) return false;

    return std::equal(a.begin(), a.end(), b.begin(),
        [](char c1, char c2) {
            return std::tolower(static_cast<unsigned char>(c1)) ==
                std::tolower(static_cast<unsigned char>(c2));
        });
}

// Callback-функция для получения данных от curl
static size_t WriteCallback(void* contents, size_t size, size_t nmemb, string* userp)
{
    size_t realsize = size * nmemb;
    userp->append(static_cast<char*>(contents), realsize);
    return realsize;
}

// Функция загрузки HTML-страницы
int DownloadHTML(const string& url, string* html)
{
    if (!html) {
        cerr << "Error: pointer html == nullptr\n";
        return 1;
    }

    // Очищаем строку на всякий случай
    html->clear();

    CURL* curl = curl_easy_init();
    if (!curl) {
        cerr << "Initialization failure curl\n";
        return 1;
    }

    // Основные настройки
    curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteCallback);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, html);
    
    //Mozilla / 5.0 (compatible; SimpleBot / 1.0)
    // Полезные и часто рекомендуемые настройки
    curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, 1L);       // следовать редиректам
    curl_easy_setopt(curl, CURLOPT_MAXREDIRS, 10L);           // максимум редиректов
    curl_easy_setopt(curl, CURLOPT_USERAGENT, "Chrome / 91.0.4472.114");
    curl_easy_setopt(curl, CURLOPT_ACCEPT_ENCODING, "gzip, deflate"); // поддержка сжатия

    // Таймауты (рекомендуется)
    curl_easy_setopt(curl, CURLOPT_CONNECTTIMEOUT, 15L);     // 15 секунд на соединение
    curl_easy_setopt(curl, CURLOPT_TIMEOUT, 30L);             // общий таймаут 30 секунд

    // Выполнение запроса
    CURLcode res = curl_easy_perform(curl);

    // Проверка результата
    if (res != CURLE_OK) {
        cerr << "curl_easy_perform() failed: " << curl_easy_strerror(res) << '\n';
        curl_easy_cleanup(curl);
        return 1;
    }

    // Успешно
    cout << "Loaded successful " << html->length() << " bytes\n";

    curl_easy_cleanup(curl);
    return 0;
}

// Function to trim whitespace from both ends of a string
std::string trim(const std::string& str) {
    size_t start = str.find_first_not_of(" \t\n\r");
    size_t end = str.find_last_not_of(" \t\n\r");

    if (start == std::string::npos || end == std::string::npos) {
        return ""; // Return an empty string if no non-whitespace characters are found
    }

    return str.substr(start, end - start + 1);
}

std::vector<std::string> get_soundcloud_track_urls(
    const std::string& user_id,          // "4521252"
    const std::string& client_id,
    size_t limit_per_page = 50)
{
    std::vector<std::string> urls;
    std::string response;
    std::string base_url = "https://api-v2.soundcloud.com/users/" + user_id + "/tracks";
    bool has_more = true;
    size_t offset = 0;
    while (has_more) {
        std::string url = base_url + "?client_id=" + client_id +
            "&limit=" + std::to_string(limit_per_page) +
            "&offset=" + std::to_string(offset);
        DownloadHTML(url, &response);
        if (response.empty())
        {
            cerr << "ERROR: response is empty!" << endl;
            return urls;
        }
        size_t start = 0;
        if (response.size() >= 3 &&
            static_cast<unsigned char>(response[0]) == 0xEF &&
            static_cast<unsigned char>(response[1]) == 0xBB &&
            static_cast<unsigned char>(response[2]) == 0xBF)
        {
            start = 3;
            std::cout << "BOM found and deleted!\n";
        }
        if (response.empty())
        {
            cerr << "ERROR: Data is empty!" << endl;
            return urls;
        }
        rapidjson::Document Doc;

        Doc.Parse<rapidjson::kParseStopWhenDoneFlag>(
            response.data() + start,
            response.size() - start
        );
        if (Doc.HasParseError()) {
            // Можно вывести подробности ошибки (очень полезно при отладке)
            std::cerr << "JSON is not valid. Error: "
                << rapidjson::GetParseError_En(Doc.GetParseError())
                << " (position " << Doc.GetErrorOffset() << ")\n";

            // Примеры типичных сообщений:
            //   "The document is empty."
            //   "Missing a name for object member."
            //   "Incorrect syntax"
            //   "The value is invalid."

            return urls;
        }
        auto err = Doc.HasMember("collection");
        if (!Doc.HasMember("collection"))
            if (!Doc["collection"].IsArray() || (Doc.HasMember("collection") && Doc["collection"].Empty() && Doc["collection"].IsArray()))
            {
                cout << "Collection is empty!" << endl;
                break;
            }
        for (const auto& track : Doc["collection"].GetArray())
        {
            if (track.HasMember("permalink_url") && track["permalink_url"].IsString())
            {

                urls.push_back(track["permalink_url"].GetString());
            }
            has_more = Doc.HasMember("next_href") && !Doc["next_href"].IsNull();
            offset += limit_per_page;

            std::cout << "Recieved " << urls.size() << " tracks...\n";
            std::this_thread::sleep_for(std::chrono::milliseconds(1200));
        }
    }
    return urls;
}

std::string exec(const char* cmd) {
    // Буфер для чтения
    char buffer[128];
    std::string result = "";
    // Открываем поток для чтения вывода команды
    std::unique_ptr<FILE, decltype(&pclose)> pipe(popen(cmd, "r"), pclose);
    if (!pipe) {
        throw std::runtime_error("popen() failed!");
    }
    // Читаем построчно и добавляем в строку
    while (fgets(buffer, sizeof(buffer), pipe.get()) != nullptr) {
        result += buffer;
    }
    return result;
}

int WriteToCSV(string NameFile, vector<pair<string, string>>* List)
{
    ofstream FileStream(NameFile);
    if (List->empty())
    {
        cerr << "Data is empty!" << endl;
        return -1;
    }
    FileStream << "Name,Links" << endl;
    for (auto& Pair : *List)
    {
        string Buff = Pair.first;
        if (Buff[Buff.size() - 1] == '\n')
            Buff.erase(Buff.size() - 1, 1);
        FileStream << Buff << ",";
        Buff = Pair.second;
        if (Buff[Buff.size() - 1] == '\n')
            Buff.erase(Buff.size() - 1, 1);
        FileStream << Buff << endl;
    }
    return 0;
}

void find_buttons_and_images(const GumboNode* node)
{
    if (node->type != GUMBO_NODE_ELEMENT)
        return;

    const GumboElement& elem = node->v.element;
    GumboTag tag = elem.tag;

    // -----------------------
    //        Кнопки
    // -----------------------
    if (tag == GUMBO_TAG_BUTTON)
    {
        std::cout << "Found <button>";

        // Попробуем достать текст кнопки (самый простой случай)
        const GumboVector* children = &elem.children;
        if (children->length > 0)
        {
            const GumboNode* child = static_cast<GumboNode*>(children->data[0]);
            if (child->type == GUMBO_NODE_TEXT)
            {
                std::cout << " → text: \"" << child->v.text.text << "\"";
            }
        }
        std::cout << "\n";
    }
    else if (tag == GUMBO_TAG_INPUT)
    {
        const GumboAttribute* type_attr = gumbo_get_attribute(&elem.attributes, "type");
        if (type_attr &&
            (iequals(type_attr->value, "button") == 0 ||
      
                iequals(type_attr->value, "submit") == 0 ||
                iequals(type_attr->value, "reset") == 0))
        {
            const GumboAttribute* value_attr = gumbo_get_attribute(&elem.attributes, "value");
            std::cout << "Found <input type=\"" << (type_attr->value ? type_attr->value : "?")
                << "\">";

            if (value_attr)
                std::cout << " → value: \"" << value_attr->value << "\"";

            std::cout << "\n";
        }
    }

    // -----------------------
    //        Картинки
    // -----------------------
    if (tag == GUMBO_TAG_IMG)
    {
        const GumboAttribute* src = gumbo_get_attribute(&elem.attributes, "src");
        const GumboAttribute* alt = gumbo_get_attribute(&elem.attributes, "alt");

        std::cout << "Found <img>";

        if (src)
            std::cout << " src=\"" << src->value << "\"";
        else
            std::cout << " src=(absent)";

        if (alt)
            std::cout << " alt=\"" << alt->value << "\"";

        std::cout << "\n";
    }

    // Рекурсивный обход детей
    const GumboVector* children = &elem.children;
    for (unsigned int i = 0; i < children->length; ++i)
    {
        find_buttons_and_images(static_cast<const GumboNode*>(children->data[i]));
    }
}

// Рекурсивный поиск всех ссылок <a href="...">
void find_all_links(const GumboNode* node, vector<string>& links)
{
    if (node->type != GUMBO_NODE_ELEMENT)
        return;

    if (node->v.element.tag == GUMBO_TAG_A)
    {
        const GumboAttribute* href = gumbo_get_attribute(&node->v.element.attributes, "href");
        if (href)
            links.push_back(href->value);
    }

    const GumboVector* children = &node->v.element.children;
    for (unsigned int i = 0; i < children->length; ++i)
    {
        find_all_links(static_cast<GumboNode*>(children->data[i]), links);
    }
}
std::string readAll(const std::string& fileName)
{
    std::ifstream ifs;
    ifs.open(fileName);
    ifs.seekg(0, std::ios::end);
    size_t length = ifs.tellg();
    ifs.seekg(0, std::ios::beg);
    std::string buff(length, 0);
    ifs.read(&buff[0], length);
    ifs.close();

    return buff;
}
