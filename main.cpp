#include <QApplication>
#include <QIcon>
#include <QMainWindow>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLineEdit>
#include <QTextBrowser>
#include <QLabel>
#include <QProcess>
#include <QRegularExpression>
#include <QStyle>

// ==============================================================================
// CLASS INTERFACE DECLARATION
// ==============================================================================
class PwssBrowser : public QMainWindow {
    Q_OBJECT

public:
    PwssBrowser(QWidget *parent = nullptr);

private slots:
    void navigate();

private:
    QString renderMarkdownToHtml(const QString &markdown);
    void updateStatus(const QString &text, const QString &fgColor, const QString &bgColor);

    QLineEdit *addressBar;
    QTextBrowser *viewFrame;
    QLabel *statusBar;
};

// ==============================================================================
// CLASS IMPLEMENTATION
// ==============================================================================
PwssBrowser::PwssBrowser(QWidget *parent) : QMainWindow(parent) {
    setWindowTitle("PWSS Sovereign Browser");
    resize(850, 650);

    QWidget *centralWidget = new QWidget(this);
    QVBoxLayout *mainLayout = new QVBoxLayout(centralWidget);
    mainLayout->setContentsMargins(12, 12, 12, 12);
    mainLayout->setSpacing(10);

    QHBoxLayout *topBar = new QHBoxLayout();
    addressBar = new QLineEdit(this);
    addressBar->setPlaceholderText("Enter identity shortcut (e.g., blog, zai1208/mobile-setup)...");
    topBar->addWidget(addressBar);
    mainLayout->addLayout(topBar);

    viewFrame = new QTextBrowser(this);
    
    // Explicit high-fidelity monospace cascading fallbacks matching modern terminal specs
    QFont monoFont;
    monoFont.setFamilies({ "JetBrains Mono", "Fira Code", "Ghostty", "SF Mono", "Hack", "Monospace" });
    monoFont.setPointSize(11);
    monoFont.setStyleHint(QFont::Monospace);
    viewFrame->setFont(monoFont);
    mainLayout->addWidget(viewFrame);

    statusBar = new QLabel("Ready to Route", this);
    statusBar->setFrameStyle(QFrame::Panel | QFrame::Sunken);
    statusBar->setContentsMargins(8, 5, 8, 5);
    mainLayout->addWidget(statusBar);

    setCentralWidget(centralWidget);

    // Deep Catppuccin-esque palette that matches your setup's border radii and lines
    setStyleSheet(
        "QMainWindow { background-color: #1e1e2e; }"
        "QLineEdit { background-color: #313244; color: #cdd6f4; border: 1px solid #45475a; "
        "            border-radius: 6px; padding: 8px; font-family: 'JetBrains Mono', 'Fira Code', monospace; font-size: 13px; }"
        "QLineEdit:focus { border: 1px solid #cba6f7; }"
        "QTextBrowser { background-color: #181825; color: #cdd6f4; border: 1px solid #313244; "
        "               border-radius: 8px; padding: 16px; }"
        "QLabel { background-color: #313244; color: #a6adc8; border-radius: 6px; font-family: 'JetBrains Mono', monospace; font-size: 12px; }"
    );

    connect(addressBar, &QLineEdit::returnPressed, this, &PwssBrowser::navigate);
}

// Fine-grained regex parsing suite to build highly structured pages
QString PwssBrowser::renderMarkdownToHtml(const QString &markdown) {
    QString html = markdown.toHtmlEscaped();
    QStringList lines = html.split("\n");
    QString parsedHtml;
    
    bool inList = false;

    for (QString line : lines) {
        // Handle Horizontal Rules (----)
        if (line.trimmed().startsWith("---") || line.trimmed().startsWith("——-")) {
            if (inList) { parsedHtml += "</ul>\n"; inList = false; }
            parsedHtml += "<hr style='border: 0; border-top: 1px solid #45475a; margin: 16px 0;'>\n";
            continue;
        }

        // Handle Headings (###, ##, #)
        if (line.startsWith("### ")) {
            if (inList) { parsedHtml += "</ul>\n"; inList = false; }
            parsedHtml += "<h3 style='color: #f9e2af; margin-top: 14px; margin-bottom: 6px;'>" + line.mid(4) + "</h3>\n";
            continue;
        }
        if (line.startsWith("## ")) {
            if (inList) { parsedHtml += "</ul>\n"; inList = false; }
            parsedHtml += "<h2 style='color: #cba6f7; margin-top: 18px; margin-bottom: 8px; border-bottom: 1px solid #313244; padding-bottom: 4px;'>" + line.mid(3) + "</h2>\n";
            continue;
        }
        if (line.startsWith("# ")) {
            if (inList) { parsedHtml += "</ul>\n"; inList = false; }
            parsedHtml += "<h1 style='color: #89b4fa; margin-top: 22px; margin-bottom: 10px;'>" + line.mid(2) + "</h1>\n";
            continue;
        }

        // Handle Bullet Points
        if (line.trimmed().startsWith("* ") || line.trimmed().startsWith("- ")) {
            if (!inList) { parsedHtml += "<ul style='color: #cdd6f4; margin-left: 15px; padding-left: 5px;'>\n"; inList = true; }
            QString itemContent = line.trimmed().mid(2);
            parsedHtml += "<li style='margin-bottom: 4px;'>" + itemContent + "</li>\n";
            continue;
        }

        // Blank lines close lists
        if (line.trimmed().isEmpty()) {
            if (inList) { parsedHtml += "</ul>\n"; inList = false; }
            parsedHtml += "<br>\n";
            continue;
        }

        // Standard Paragraph Line
        if (inList) { parsedHtml += "</ul>\n"; inList = false; }
        parsedHtml += "<p style='line-height: 1.5; margin-bottom: 8px;'>" + line + "</p>\n";
    }
    if (inList) { parsedHtml += "</ul>\n"; }

    // Inline Emphasis Formatting Transformations (Bold, Italic, Code Ticks)
    // We unescape specific tags carefully to ensure inline rendering operates flawlessly
    parsedHtml.replace(QRegularExpression("\\*\\*(.*?)\\*\\*"), "<b>\\1</b>");
    parsedHtml.replace(QRegularExpression("\\*(.*?)\\*"), "<i>\\1</i>");
    parsedHtml.replace(QRegularExpression("`(.*?)`"), "<code style='background-color: #313244; color: #f5e0dc; padding: 2px 4px; border-radius: 3px;'>\\1</code>");

    return parsedHtml;
}

void PwssBrowser::navigate() {
    QString input = addressBar->text().trimmed();
    if (input.isEmpty()) return;

    updateStatus("Resolving 3-Tier Network Identity...", "#f9e2af", "#1e1e2e");
    viewFrame->clear();

    QProcess *fetchProc = new QProcess(this);
    fetchProc->start("pwss-fetch-https", QStringList() << input);
    fetchProc->waitForFinished();

    if (fetchProc->exitCode() != 0) {
        QString errOutput = fetchProc->readAllStandardError().trimmed();
        viewFrame->setHtml(QString("<span style='color:#f38ba8; font-weight:bold;'>[RESOLUTION ERROR]</span><br><br><pre style='color:#a6adc8; font-family:monospace;'>%1</pre>")
                           .arg(errOutput.toHtmlEscaped()));
        updateStatus("Resolution Failed (404 / Collision)", "#f38ba8", "#1e1e2e");
        fetchProc->deleteLater();
        return;
    }

    QString rawOutput = fetchProc->readAllStandardOutput().trimmed();
    fetchProc->deleteLater();

    QStringList tokens = rawOutput.split(" ", Qt::SkipEmptyParts);
    if (tokens.size() < 2) {
        viewFrame->setHtml("<span style='color:#f38ba8;'>System Error: Backend framework returned invalid parameters.</span>");
        updateStatus("Internal Pipe Malformed", "#f38ba8", "#1e1e2e");
        return;
    }

    QString targetUrl = tokens.at(0);
    QString fingerprint = tokens.at(1);

    updateStatus("Streaming Cryptographic Stream Assets...", "#89b4fa", "#1e1e2e");
    
    QProcess curlProc;
    QProcess coreProc;

    curlProc.setStandardOutputProcess(&coreProc);
    curlProc.start("curl", QStringList() << "-sL" << targetUrl);
    coreProc.start("pwss-core", QStringList() << fingerprint);

    curlProc.waitForFinished();
    coreProc.waitForFinished();

    QString payloadContent = coreProc.readAllStandardOutput();
    QString coreErrors = coreProc.readAllStandardError();

    if (coreProc.exitCode() != 0 || payloadContent.isEmpty()) {
        viewFrame->setHtml(QString("<span style='color:#f38ba8; font-weight:bold;'>[CRYPTOGRAPHIC BREACH] Security Verification Blocked Execution!</span><br><br><pre style='color:#a6adc8; font-family:monospace;'>%1</pre>")
                           .arg(coreErrors.toHtmlEscaped()));
        updateStatus("SECURITY CRISIS: INVALID CONTENT SIGNATURE", "#f38ba8", "#11111b");
        return;
    }

    // Process our text source content through the markdown rendering pipeline
    QString parsedContentHtml = renderMarkdownToHtml(payloadContent);

    // Build our native security verification dynamic layout block
    QString htmlBanner = QString(
        "<div style='margin-top: 24px; padding: 12px; border: 1px solid #a6e3a1; background-color: #1e2e24; border-radius: 6px; font-family: monospace;'>"
        "<b style='color:#a6e3a1;'>⚡ SOVEREIGN IDENTITY VERIFIED</b><br>"
        "<div style='color:#cdd6f4; margin-top: 4px;'><b>FPR:</b> %1</div>"
        "<div style='color:#bac2de; margin-top: 2px;'><b>SRC:</b> %2</div>"
        "</div>"
    ).arg(fingerprint, targetUrl);

    // Replace the template macro with our clean native container
    // Since we handle typography explicitly, we replace the string inside the HTML context directly
    QString targetBannerPlaceholder = "{{&lt; pgp_banner &gt;}}";
    if (parsedContentHtml.contains(targetBannerPlaceholder)) {
        parsedContentHtml.replace(targetBannerPlaceholder, htmlBanner);
    } else {
        parsedContentHtml += htmlBanner;
    }

    // Inject the structured layout directly into the canvas
    viewFrame->setHtml(parsedContentHtml);
    updateStatus(QString(" Verified Sovereign Identity Pinned: %1").arg(fingerprint.left(12) + "..."), "#a6e3a1", "#11111b");
}

void PwssBrowser::updateStatus(const QString &text, const QString &fgColor, const QString &bgColor) {
    statusBar->setText(text);
    statusBar->setStyleSheet(QString("background-color: %1; color: %2; font-weight: bold;").arg(bgColor, fgColor));
}

// ==============================================================================
// ENTRY SYSTEM INITIALIZATION
// ==============================================================================
// Ensure #include <QIcon> is added up top if not present

int main(int argc, char *argv[]) {
    QApplication app(argc, argv);
    
    // Explicitly pin the application name and desktop theme icon target
    app.setApplicationName("PWSS Browser");
    app.setWindowIcon(QIcon::fromTheme("pwss-browser"));
    
    PwssBrowser browser;
    browser.show();
    return app.exec();
}

#include "main.moc"
