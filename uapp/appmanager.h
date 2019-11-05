#ifndef APPMANAGER_H
#define APPMANAGER_H

#include <QObject>
#include <QQuickImageProvider>

#include <future>

#include <uengine.h>

#include "scene.h"

const std::map<QString, URendererType> renderer_type_map { {"BDPT", URendererType::BDPT} };
const std::map<QString, URgbFormat> rgb_format_map { {"sRGB", URgbFormat::sRGB} };

class AppManager : public QObject, public QQuickImageProvider
{
    Q_OBJECT
    Q_PROPERTY(QString previewImg READ getPreviewImg NOTIFY previewImgChanged)
    Q_PROPERTY(QString logText READ getLogText NOTIFY logTextChanged)
    Q_PROPERTY(QVariantList rendererTypes READ getRendererTypes NOTIFY rendererTypesChanged)

    Q_PROPERTY(QString currPass READ getCurrPass NOTIFY currPassChanged)
    Q_PROPERTY(QString avgPassTime READ getAvgPassTime NOTIFY avgPassTimeChanged)
    Q_PROPERTY(QString numThreads READ getNumThreads NOTIFY numThreadsChanged)

    Q_PROPERTY(QString status READ getStatus NOTIFY statusChanged)
    Q_PROPERTY(bool running READ getRunning NOTIFY statusChanged)
    Q_PROPERTY(QString progress READ getProgress NOTIFY progressChanged)

    Q_PROPERTY(double gamma READ getGamma NOTIFY gammaChanged)
    Q_PROPERTY(QVariantList rgbFormats READ getRgbFormats NOTIFY rgbFormatsChanged)

    Q_PROPERTY(QString imgWidth READ getImgWidth NOTIFY imgWidthChanged)
    Q_PROPERTY(QString imgHeight READ getImgHeight NOTIFY imgHeightChanged)
    Q_PROPERTY(QString pixelSubdiv READ getPixelSubdiv NOTIFY pixelSubdivChanged)
    Q_PROPERTY(QString lensSubdiv READ getLensSubdiv NOTIFY lensSubdivChanged)
    Q_PROPERTY(QString lensSize READ getLensSize NOTIFY lensSizeChanged)
    Q_PROPERTY(QString focusPlane READ getFocusPlane NOTIFY focusPlaneChanged)
    Q_PROPERTY(QString minDepth READ getMinDepth NOTIFY minDepthChanged)
    Q_PROPERTY(QString rendererType READ getRendererType NOTIFY rendererTypeChanged)

public:
    explicit AppManager(QObject *parent = nullptr);
    bool initialize();
    void updateProgress(double);

    QPixmap requestPixmap(const QString &id, QSize *size, const QSize &requestedSize);

    Q_INVOKABLE void newRendering(QList<QString> params);
    Q_INVOKABLE void loadRendering(QString filename);
    Q_INVOKABLE void loadScene(QString filename);
    Q_INVOKABLE void refreshScene();
    Q_INVOKABLE void saveRendering(QString filename);
    Q_INVOKABLE void saveImage(QString filename);
    Q_INVOKABLE void startRendering(QString num_threads);
    Q_INVOKABLE void stopRendering();
    Q_INVOKABLE void onClosing();

    QString getPreviewImg() const;
    QString getLogText() const;
    QVariantList getRendererTypes() const;

    QString getCurrPass() const;
    QString getAvgPassTime() const;
    QString getNumThreads() const;

    QString getStatus() const;
    bool getRunning() const;
    QString getProgress() const;

    double getGamma() const;
    Q_INVOKABLE void setGamma(const double&);
    QVariantList getRgbFormats() const;
    Q_INVOKABLE void setRgbFormat(const QString&);

    QString getImgWidth() const;
    QString getImgHeight() const;
    QString getPixelSubdiv() const;
    QString getLensSubdiv() const;
    QString getLensSize() const;
    QString getFocusPlane() const;
    QString getMinDepth() const;
    QString getRendererType() const;

signals:
    void previewImgChanged();
    void logTextChanged();
    void rendererTypesChanged();

    void currPassChanged();
    void avgPassTimeChanged();
    void numThreadsChanged();

    void statusChanged();
    void progressChanged();

    void gammaChanged();
    void rgbFormatsChanged();

    void imgWidthChanged();
    void imgHeightChanged();
    void pixelSubdivChanged();
    void lensSubdivChanged();
    void lensSizeChanged();
    void focusPlaneChanged();
    void minDepthChanged();
    void rendererTypeChanged();

public slots:

private:
    void log(const std::string&);
    void logInfo(const std::string&);
    void logDebug(const std::string&);
    void logError(const std::string&);
    void updateParameterLabels(const URenderParameters&);

    void fetchRgbImageFromEngine();
    void renderLoop();

    double m_total_time;
    size_t m_progress;
    std::atomic<bool> m_running;
    std::future<void> m_render_future;

    double m_gamma;
    URgbFormat m_rgb_format;
    std::mutex m_img_rgb_mutex;
    std::vector<uint8_t> m_img_rgb;

    QString m_scene_filename;
    std::shared_ptr<Scene> m_scene;

    /*---properties---*/
    QString m_log_text;

    size_t m_curr_pass;
    double m_avg_pass_time;
    size_t m_num_threads;

    size_t m_img_width;
    size_t m_img_height;
    size_t m_pixel_subdiv;
    size_t m_lens_subdiv;
    double m_lens_size;
    double m_focus_plane;
    size_t m_min_depth;
    URendererType m_renderer_type;
};

#endif // APPMANAGER_H
