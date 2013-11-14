package net.t32leaves.pebblelightlogger;

import android.content.Context;
import android.util.Log;

import com.getpebble.android.kit.PebbleKit;
import com.getpebble.android.kit.util.PebbleDictionary;

import java.util.UUID;

/**
 * Created by blacki on 11/10/13.
 */
public class LogMessageReceiver extends PebbleKit.PebbleDataReceiver {

    public LogMessageReceiver() {
        super(UUID.fromString("5BE44F1D-D262-4EA6-AA30-EDBE01E38A00"));
    }

    @Override
    public void receiveData(Context context, int transactionId, PebbleDictionary data) {
        PebbleKit.sendAckToPebble(context, transactionId);

        LightLoggerDB db = new LightLoggerDB(context);
        try {
            long tag = data.getInteger(0x00);
            long msgTimestamp = data.getInteger(0x01);
            db.addEvent(tag, msgTimestamp, data.getUnsignedInteger(0x02));

            Log.d("LightLog", "TAG=" + tag + " WHEN=" + msgTimestamp + " VALUE=" + data.getUnsignedInteger(0x02));
        } finally {
            db.close();
        }
    }

}
