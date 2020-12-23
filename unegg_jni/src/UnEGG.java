import java.nio.charset.StandardCharsets;

public class UnEGG {
    static {
        System.loadLibrary("unegg_jni");
    }

    /*
        Retrieves last-error code value.

        Return value is last-error code.
        0 - Succeeded
        79 - Bad Format
        80 - No Interface
        81 - File Not Found
        82 - Out of Memory
        83 - End of File
        84 - Unsupported
        85 - Invalid Parameters
        86 - Invalid Flags
        87 - Invalid Password
        88 - Open Failed
        89 - Undefined
    */
    private native int _GetLastResult();

    /*
        Opens a archive file.

        utf8path - The name of the archive file to be opened.
                   This must be UTF-8 encoding.

        Return value
        If the function succeeds, the return value is an open handle to the specified archive file.
        If the function fails, the return value is 0. To get extended error information, call _GetLastResult.
    */
    private native int _OpenArchive(byte[] utf8path);

    /*
        Closes an open archive handle.

        handle - A valid handle to an open archive.
    */
    private native void _CloseArchive(int handle);

    /*
        Gets the number of items stored in the archive object.

        handle - A handle to an open archive.

        Return value
        If the function succeeds, the return value is the number of items stored in the archive object.
        If the function fails, the return value is 0. To get extended error information, call _GetLastResult.
    */
    private native int _GetNumberOfItems(int handle);

    /*
        Retrieves name of specified item stored in the archive object.

        handle - A handle to an open archive.
        index - The index of the item to retrieve.

        Return value
        If the function succeeds, the return value is the name of specified item.
        If the function fails, the return value is empty string. To get extended error information, call _GetLastResult.
    */
    private native String _GetItemName(int handle, int index);

    /*
        Extract all items stored in the archive object.

        handle - A handle to an open archive.
        utf8dest - The path to extracted.
                   This must be UTF-8 encoding.
        utf8password - The password of archive.
                       This must be UTF-8 encoding.

        Return value
        If the function succeeds, the return value is true.
        If the function fails, the return value is false. To get extended error information, call _GetLastResult.
    */
    private native boolean _Extract(int handle, byte[] utf8dest, byte[] utf8password);

    public static void main(String[] args) {
        if (args.length < 2) {
            System.out.println("java UnEGG [archive] [destination] (password)");
        } else {
            UnEGG ob = new UnEGG();
            if (ob.OpenArchive(args[0])) {
                int count = ob.GetNumberOfItems();
                for (int i = 0; i < count; i++) {
                    String name = ob.GetItemName(i);
                    if (name.isEmpty()) {
                        System.out.println(ob.GetLastResult());
                    } else {
                        System.out.println(name);
                    }
                }
                if (!ob.Extract(args[1], ((args.length > 2) ? args[2] : ""))) {
                    System.out.println(ob.GetLastResult());
                }
                ob.CloseArchive();
            } else {
                System.out.println(ob.GetLastResult());
            }
        }
    }

    public int GetLastResult() {
        return _GetLastResult();
    }

    public boolean OpenArchive(String path) {
        handle_ = _OpenArchive(
                path.getBytes(StandardCharsets.UTF_8)
        );
        return handle_ != 0;
    }

    public void CloseArchive() {
        _CloseArchive(handle_);
    }

    public int GetNumberOfItems() {
        return _GetNumberOfItems(handle_);
    }

    public String GetItemName(int index) {
        return _GetItemName(handle_, index);
    }

    public boolean Extract(String destination, String password) {
        return _Extract(handle_,
                destination.getBytes(StandardCharsets.UTF_8),
                password.getBytes(StandardCharsets.UTF_8)
        );
    }

    private int handle_;
}
