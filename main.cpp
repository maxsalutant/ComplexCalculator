#define sscanf_s sscanf
#define strcpy_s(dest, src) strcpy(dest, src)
#define strncpy_s(dest, src, count) strncpy(dest, src, count)
#include "imgui.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_opengl3.h"
#include "ComplexOperations.h"
#include <GLFW/glfw3.h>
#include <cstdio>
#include <cstring>

// Кроссплатформенная версия без _s функций
Complex ParseComplex(const char* str) {
    char buf[64];
    std::strncpy(buf, str, sizeof(buf) - 1);
    buf[sizeof(buf) - 1] = '\0';

    for (int i = 0; buf[i]; i++) {
        if (buf[i] == ',') buf[i] = '.';
    }

    double r = 0.0, im = 0.0;
    if (std::strchr(buf, 'i') != nullptr) {
        if (std::sscanf(buf, "%lf%lfi", &r, &im) != 2) {
            r = 0.0;
            if (std::strcmp(buf, "i") == 0 || std::strcmp(buf, "+i") == 0) im = 1.0;
            else if (std::strcmp(buf, "-i") == 0) im = -1.0;
            else std::sscanf(buf, "%lfi", &im);
        }
    } else {
        std::sscanf(buf, "%lf", &r);
    }
    return Complex(r, im);
}

int main(int, char**) {
    if (!glfwInit())
        return 1;

    // Настройка версии OpenGL 3.0 / GLSL 130 (стандарт для Linux)
    const char* glsl_version = "#version 130";
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 0);

    GLFWwindow* window = glfwCreateWindow(1280, 800, "Complex number calculator", nullptr, nullptr);
    if (window == nullptr)
        return 1;
    glfwMakeContextCurrent(window);
    glfwSwapInterval(1); // Включение VSync

    // Инициализация Dear ImGui
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO(); (void)io;
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;

    ImGuiStyle& style = ImGui::GetStyle();
    ImGui::StyleColorsDark();
    style.FrameRounding = 3.0f;
    style.GrabRounding = 3.0f;

    // Подключение бэкендов GLFW + OpenGL3
    ImGui_ImplGlfw_InitForOpenGL(window, true);
    ImGui_ImplOpenGL3_Init(glsl_version);

    ImVec4 clear_color = ImVec4(0.45f, 0.55f, 0.60f, 1.00f);

    // Главный цикл
    while (!glfwWindowShouldClose(window)) {
        glfwPollEvents();

        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();

        // Интерфейс калькулятора
        {
            static Complex counter;
            static char y[128] = "";

            ImGui::Begin("Complex number calculator");

            ImGui::SetWindowFontScale(1.2f);
            ImGui::TextWrapped("Enter a complex number in algebraic form (a + bi)");

            ImGui::SetWindowFontScale(1.8f);
            ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(10.0f, 13.0f));

            float inputWidth = ImGui::GetContentRegionAvail().x - 320.0f;
            if (inputWidth < 150.0f) inputWidth = 150.0f;
            ImGui::SetNextItemWidth(inputWidth);

            if (ImGui::IsWindowAppearing()) {
                ImGui::SetKeyboardFocusHere();
            }
            bool enterPressed = ImGui::InputText("##input", y, IM_COUNTOF(y), ImGuiInputTextFlags_EnterReturnsTrue);

                       // Ловим нажатие Enter / Numpad Enter, даже если фокус ушёл с поля ввода после клика по кнопкам
            if (ImGui::IsWindowFocused(ImGuiFocusedFlags_RootAndChildWindows) &&
                (ImGui::IsKeyPressed(ImGuiKey_Enter) || ImGui::IsKeyPressed(ImGuiKey_KeypadEnter))) {
                enterPressed = true;
            }

            ImGui::PopStyleVar(); // Возвращаем стандартные отступы

            // Ставим кнопку "=" строго в ту же строку
            ImGui::SameLine();
            ImGui::SetWindowFontScale(2.5f);

            // 2. Срабатывание по клику на "=" ИЛИ по нажатию Enter
            if (ImGui::Button("=", ImVec2(100, 50)) || enterPressed) {
                // Убираем пробелы, если выражение вводили с клавиатуры
                char cleanY[128];
                int j = 0;
                for (int i = 0; y[i] != '\0' && j < 127; i++) {
                    if (y[i] != ' ') cleanY[j++] = y[i];
                }
                cleanY[j] = '\0';

                char leftStr[64] = "", rightStr[64] = "";
                char op = 0;
                bool parsed = false;

                // Вариант А: Ищем формат "(число1) знак (число2)" уже в очищенной строке cleanY
                if (sscanf_s(cleanY, "(%63[^)])%c(%63[^)])", leftStr, (unsigned)sizeof(leftStr), &op, 1, rightStr, (unsigned)sizeof(rightStr)) == 3) {
                    parsed = true;
                }
                // Вариант Б: Ввод с клавиатуры без скобок (например, "3+4i-3+4i" или "5+2i")
                else {
                    const char* posI = strchr(cleanY, 'i');
                    if (posI != nullptr && *(posI + 1) != '\0') {
                        // Если у первого числа есть мнимая часть, знак операции идёт сразу после первой 'i'
                        int splitIdx = (int)(posI - cleanY) + 1;
                        op = cleanY[splitIdx];
                        if ((op == '+' || op == '-' || op == '*' || op == '/') && splitIdx < 64) {
                            strncpy_s(leftStr, cleanY, splitIdx);
                            strcpy_s(rightStr, cleanY + splitIdx + 1);
                            parsed = true;
                        }
                    }
                    else {
                        // Если первое число чисто действительное (без 'i'), ищем первый знак операции после первой цифры
                        for (int i = 1; cleanY[i] != '\0'; i++) {
                            if (cleanY[i] == '+' || cleanY[i] == '-' || cleanY[i] == '*' || cleanY[i] == '/') {
                                op = cleanY[i];
                                if (i < 64) {
                                    strncpy_s(leftStr, cleanY, i);
                                    strcpy_s(rightStr, cleanY + i + 1);
                                    parsed = true;
                                }
                                break;
                            }
                        }
                    }
                }

                if (parsed) {
                    Complex c1 = ParseComplex(leftStr);
                    Complex c2 = ParseComplex(rightStr);

                    if (op == '+') counter = c1 + c2;
                    else if (op == '-') counter = c1 - c2;
                    else if (op == '*') counter = c1 * c2;
                    else if (op == '/') counter = c1 / c2;
                }
            }

            // Вывод результата справа (полное комплексное число)
            ImGui::SetWindowFontScale(1.5f);
            ImGui::SameLine();
            if (counter.getImag() >= 0)
                ImGui::Text("%.2f + %.2fi", counter.getReal(), counter.getImag());
            else
                ImGui::Text("%.2f - %.2fi", counter.getReal(), -counter.getImag());

            // --- Адаптивные кнопки снизу ---
            ImGui::Spacing();
            ImGui::Separator();
            ImGui::Spacing();

            float availWidth = ImGui::GetContentRegionAvail().x;
            float spacing = ImGui::GetStyle().ItemSpacing.x;
            float btnHeight = 45.0f;

            // 60% ширины окна под цифры слева, 40% под операции справа
            float leftWidth = availWidth * 0.60f - spacing;
            float rightWidth = availWidth * 0.40f;

            // 1. ЛЕВЫЙ БЛОК: 10 цифр + запятая (сетка 3 колонки)
            ImGui::BeginGroup();
            float numBtnW = (leftWidth - spacing * 2.0f) / 3.0f;
            if (numBtnW < 30.0f) numBtnW = 30.0f;

            const char* numButtons[] = {
                "7", "8", "9",
                "4", "5", "6",
                "1", "2", "3",
                "0", ","
            };

            for (int k = 0; k < 11; k++) {
                // Кнопка "0" занимает 2 колонки для ровной сетки
                float w = (k == 9) ? (numBtnW * 2.0f + spacing) : numBtnW;

                if (ImGui::Button(numButtons[k], ImVec2(w, btnHeight))) {
                    size_t len = strlen(y);
                    if (len + 1 < IM_COUNTOF(y)) {
                        y[len] = *numButtons[k];
                        y[len + 1] = '\0';
                    }
                }

                // Оставляем на той же строке все кнопки, кроме крайних правых в ряду (9, 6, 3 и запятая)
                if (k != 2 && k != 5 && k != 8 && k != 10) {
                    ImGui::SameLine();
                }
            }
            ImGui::EndGroup();

            // Переходим в правую часть окна
            ImGui::SameLine();

            // 2. ПРАВЫЙ БЛОК: операции ("/", "*", "-", "+", "(", ")", "abs", "i", "C", "<-")
            ImGui::BeginGroup();
            float opBtnW = (rightWidth - spacing) / 2.0f;
            if (opBtnW < 40.0f) opBtnW = 40.0f;

            const char* opButtons[] = {
                "/", "*",
                "-", "+",
                "(", ")",
                "abs", "i",
                "C", "<-"
            };

            for (int k = 0; k < 10; k++) {
                if (ImGui::Button(opButtons[k], ImVec2(opBtnW, btnHeight))) {
                    if (strcmp(opButtons[k], "abs") == 0) {
                        // Вызов метода модуля из класса Complex (мнимая часть становится 0)
                        Complex c = ParseComplex(y);
                        counter = Complex(c.abs(), 0.0);
                    }
                    else if (strcmp(opButtons[k], "C") == 0) {
                        // Очистка поля ввода и сброс результата в 0 + 0i
                        *y = '\0';
                        counter = Complex(0.0, 0.0);
                    }
                    else if (strcmp(opButtons[k], "<-") == 0) {
                        // Удаление последнего символа (Backspace)
                        size_t len = strlen(y);
                        if (len > 0) {
                            y[len - 1] = '\0';
                        }
                    }
                    else {
                        size_t len = strlen(y);
                        if (len + 1 < IM_COUNTOF(y)) {
                            y[len] = *opButtons[k];
                            y[len + 1] = '\0';
                        }
                    }
                }

                // По 2 кнопки в строке
                if (k % 2 == 0) {
                    ImGui::SameLine();
                }
            }
            ImGui::EndGroup();

            ImGui::SetWindowFontScale(1.0f);
            ImGui::End();
        }


        // Отрисовка кадра
        ImGui::Render();
        int display_w, display_h;
        glfwGetFramebufferSize(window, &display_w, &display_h);
        glViewport(0, 0, display_w, display_h);
        glClearColor(clear_color.x * clear_color.w, clear_color.y * clear_color.w, clear_color.z * clear_color.w, clear_color.w);
        glClear(GL_COLOR_BUFFER_BIT);
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

        glfwSwapBuffers(window);
    }

    // Очистка ресурсов
    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();

    glfwDestroyWindow(window);
    glfwTerminate();

    return 0;
}
