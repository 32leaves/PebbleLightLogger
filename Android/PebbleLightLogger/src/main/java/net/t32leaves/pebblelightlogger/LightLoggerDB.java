package net.t32leaves.pebblelightlogger;

import android.content.ContentValues;
import android.content.Context;
import android.database.sqlite.SQLiteDatabase;
import android.database.sqlite.SQLiteOpenHelper;

/**
 * Created by blacki on 11/9/13.
 */
public class LightLoggerDB extends SQLiteOpenHelper {
    private static final String FIELD_TAG  = "tag";
    private static final String FIELD_TIME = "eventTime";
    private static final String FIELD_VAL  = "value";

    private static final int DATABASE_VERSION = 2;
    private static final String DATABASE_NAME = "LightLoggerDB";
    private static final String TABLE_NAME = "logdata";
    private static final String TABLE_CREATE = "CREATE TABLE " + TABLE_NAME + " (id INTEGER PRIMARY KEY, " + FIELD_TAG + " NUMBER, " + FIELD_TIME + " NUMBER," + FIELD_VAL + " NUMBER);";

    public LightLoggerDB(Context context) {
        super(context, DATABASE_NAME, null, DATABASE_VERSION);
    }

    @Override
    public void onCreate(SQLiteDatabase db) {
        db.execSQL(TABLE_CREATE);
    }

    public void addEvent(long tag, long when, long value) {
        ContentValues values = new ContentValues();
        values.put(FIELD_TAG, tag);
        values.put(FIELD_TIME, when);
        values.put(FIELD_VAL, value);

        SQLiteDatabase database = getWritableDatabase();
        database.insert(TABLE_NAME, "N", values);
    }

    @Override
    public void onUpgrade(SQLiteDatabase sqLiteDatabase, int i, int i2) {

    }
}
