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
        cerr << "Ошибка: указатель html == nullptr\n";
        return 1;
    }

    // Очищаем строку на всякий случай
    html->clear();

    CURL* curl = curl_easy_init();
    if (!curl) {
        cerr << "Не удалось инициализировать curl\n";
        return 1;
    }

    // Основные настройки
    curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteCallback);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, html);

    // Полезные и часто рекомендуемые настройки
    curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, 1L);       // следовать редиректам
    curl_easy_setopt(curl, CURLOPT_MAXREDIRS, 10L);           // максимум редиректов
    curl_easy_setopt(curl, CURLOPT_USERAGENT, "Mozilla/5.0 (compatible; SimpleBot/1.0)");
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
    cout << "Успешно загружено " << html->length() << " байт\n";

    curl_easy_cleanup(curl);
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
        std::cout << "Найдена <button>";

        // Попробуем достать текст кнопки (самый простой случай)
        const GumboVector* children = &elem.children;
        if (children->length > 0)
        {
            const GumboNode* child = static_cast<GumboNode*>(children->data[0]);
            if (child->type == GUMBO_NODE_TEXT)
            {
                std::cout << " → текст: \"" << child->v.text.text << "\"";
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
            std::cout << "Найден <input type=\"" << (type_attr->value ? type_attr->value : "?")
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

        std::cout << "Найдена <img>";

        if (src)
            std::cout << " src=\"" << src->value << "\"";
        else
            std::cout << " src=(отсутствует)";

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

