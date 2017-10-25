#ifndef UTILS_H
#define UTILS_H

class QString;

namespace SimpleScanner
{

    class Utils
    {
    public:
        Utils();
        static bool FileExists(const QString& path);
    private:
        explicit Utils(const Utils& rhs) = delete;
        Utils& operator= (const Utils& rhs) = delete;
    };

}


#endif // UTILS_H
