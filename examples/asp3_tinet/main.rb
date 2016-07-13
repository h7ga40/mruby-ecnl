# TOPPERSプロジェクト
$MAKER_CODE = "\x00\x00\xB3"

# ノードプロファイルオブジェクト
class LocalNode < ECNL::ENode
	def initialize(eojx3)
		# 動作状態
		@operation_status = "\x30"
		# Ｖｅｒｓｉｏｎ情報
		@version_information = "\x01\x0A\x01\x00"
		# 識別番号
		@identification_number = 
			# 下位通信層IDフィールド
			"\xFE" +
			# メーカーコード
			$MAKER_CODE +
			# ユニークID部(メーカー独自)
			"\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"
		# 異常内容
		@fault_content = "\x00\x00"
		# メーカーコード
		@manufacturer_code = $MAKER_CODE

		# インスタンス数
		@inst_count = "\x00\x00\x01"
		# クラス数
		@class_count = "\x00\x02"
		# インスタンスリスト
		@inst_list = "\x01\x05\xff\x01"
		# クラスリスト
		@class_list = "\x01\x05\xff"
		# アナウンスプロパティマップ
		@anno_prpmap = "\x01\xd5"
		# SETプロパティマップ
		@set_prpmap = "\x01\x80"
		# GETプロパティマップ
		@get_prpmap = "\x0c\x80\x82\x83\x89\x8a\x9d\x9e\x9f\xd3\xd4\xd6\xd7"
		# プロパティ定義
		eprpinib_table = [
			ECNL::EProperty.new(0x80, (ECNL::EPC_RULE_SET | ECNL::EPC_RULE_GET), @operation_status.length, :@operation_status, :onoff_prop_set, :data_prop_get),
			ECNL::EProperty.new(0x82, (ECNL::EPC_RULE_GET), @version_information.length, :@version_information, :data_prop_set, :data_prop_get),
			ECNL::EProperty.new(0x83, (ECNL::EPC_RULE_GET), @identification_number.length, :@identification_number, :data_prop_set, :data_prop_get),
			ECNL::EProperty.new(0x89, (ECNL::EPC_RULE_GET), @fault_content.length, :fault_content, :@node_profile_object_fault_content_set, :data_prop_get),
			ECNL::EProperty.new(0x8A, (ECNL::EPC_RULE_GET), @manufacturer_code.length, :@manufacturer_code, :data_prop_set, :data_prop_get),
			ECNL::EProperty.new(0x9D, (ECNL::EPC_RULE_GET), @anno_prpmap.length, :@anno_prpmap, nil, :data_prop_get),
			ECNL::EProperty.new(0x9E, (ECNL::EPC_RULE_GET), @set_prpmap.length, :@set_prpmap, nil, :data_prop_get),
			ECNL::EProperty.new(0x9F, (ECNL::EPC_RULE_GET), @get_prpmap.length, :@get_prpmap, nil, :data_prop_get),
			ECNL::EProperty.new(0xD3, (ECNL::EPC_RULE_GET), @inst_count.length, :@inst_count, nil, :data_prop_get),
			ECNL::EProperty.new(0xD4, (ECNL::EPC_RULE_GET), @class_count.length, :@class_count, nil, :data_prop_get),
			ECNL::EProperty.new(0xD5, (ECNL::EPC_RULE_ANNO), @inst_list.length, :@inst_list, nil, :data_prop_get),
			ECNL::EProperty.new(0xD6, (ECNL::EPC_RULE_GET), @inst_list.length, :@inst_list, nil, :data_prop_get),
			ECNL::EProperty.new(0xD7, (ECNL::EPC_RULE_GET), @class_list.length, :@class_list, nil, :data_prop_get)
		]

		super(eojx3, eprpinib_table)
	end

	# 異常内容設定関数
	def node_profile_object_fault_content_set(prop, src)
		# サイズが2以外は受け付けない
		if (src.bytesize != 2) then
			return 0
		end

		if (prop.anno) then
			prop.set_anno(prop.exinf != src)
		end

		val = src.getbyte(0) << 8 | src.getbyte(1)

		if ((val >= 0x0) && (val <= 0x3ec)) then
			prop.set_exinf(val)
			# TODO: このの場合の処理
		# 上記以外は受け付けない
		else
			return 0
		end

		return 2
	end

	# 動作状態ON/OFF設定関数（0x30, 0x31のみ受け付け）
	def onoff_prop_set(prop, src)
		return 0
	end
end

# コントローラークラス
class ControllerObj < ECNL::EObject
	def initialize(eojx3, enod)
		# 動作状態
		@operation_status = "\x30"
		# 設置場所
		@installation_location = "\x01\x0A\x01\x00"
		# 規格Ｖｅｒｓｉｏｎ情報
		@standard_version_information = "\x00\x00C\x00"
		# 異常発生状態
		@fault_status = "\x41"
		# メーカーコード
		@manufacturer_code = $MAKER_CODE

		# アナウンスプロパティマップ
		@anno_prpmap = "\x03\x80\x81\x88"
		# SETプロパティマップ
		@set_prpmap = "\x04\x80\x81\x97\x98"
		# GETプロパティマップ
		@get_prpmap = "\x0a\x80\x81\x82\x88\x8a\x97\x98\x9d\x9e\x9f"
		# プロパティ定義
		eprpinib_table = [
			ECNL::EProperty.new(0x80, (ECNL::EPC_RULE_SET | ECNL::EPC_RULE_GET | ECNL::EPC_ANNOUNCE), @operation_status.length, :@operation_status, :onoff_prop_set, :data_prop_get),
			ECNL::EProperty.new(0x81, (ECNL::EPC_RULE_SET | ECNL::EPC_RULE_GET | ECNL::EPC_ANNOUNCE), @installation_location.length, :@installation_location, :data_prop_set, :data_prop_get),
			ECNL::EProperty.new(0x82, (ECNL::EPC_RULE_GET), @standard_version_information.length, :@standard_version_information, :data_prop_set, :data_prop_get),
			ECNL::EProperty.new(0x88, (ECNL::EPC_RULE_GET | ECNL::EPC_ANNOUNCE), @fault_status.length, :@fault_status, :alarm_prop_set, :data_prop_get),
			ECNL::EProperty.new(0x8A, (ECNL::EPC_RULE_GET), @manufacturer_code.length, :@manufacturer_code, :data_prop_set, :data_prop_get),
			ECNL::EProperty.new(0x97, (ECNL::EPC_RULE_SET | ECNL::EPC_RULE_GET), (2), nil, :time_prop_set, :time_prop_get),
			ECNL::EProperty.new(0x98, (ECNL::EPC_RULE_SET | ECNL::EPC_RULE_GET), (4), nil, :date_prop_set, :date_prop_get),
			ECNL::EProperty.new(0x9D, (ECNL::EPC_RULE_GET), @anno_prpmap.length, :@anno_prpmap, nil, :data_prop_get),
			ECNL::EProperty.new(0x9E, (ECNL::EPC_RULE_GET), @set_prpmap.length, :@set_prpmap, nil, :data_prop_get),
			ECNL::EProperty.new(0x9F, (ECNL::EPC_RULE_GET), @get_prpmap.length, :@get_prpmap, nil, :data_prop_get)
		]

		super(0x05, 0xFF, eojx3, enod, eprpinib_table)
	end

	# 動作状態ON/OFF設定関数（0x30, 0x31のみ受け付け）
	def onoff_prop_set(prop, src)
		# サイズが１以外は受け付けない
		if (src.bytesize != 1) then
			return 0
		end

		if (prop.anno) then
			prop.set_anno(prop.exinf != src)
		end

		case (src.getbyte(0))
		# ONの場合
		when 0x30 then
			prop.set_exinf(src)
			# LEDの"."をON
			digitalWrite(1, 1)
		# OFFの場合
		when 0x31 then
			prop.set_exinf(src)
			# LEDの"."をOFF
			digitalWrite(1, 0)
		# 0x30か0x31以外は受け付けない
		else
			return 0
		end

		return 1
	end

	# 異常発生状態設定関数（0x41, 0x42のみ受け付け）
	def alarm_prop_set(prop, src)
		# サイズが１以外は受け付けない
		if (src.bytesize != 1) then
			return 0
		end

		if (prop.anno) then
			prop.set_anno(prop.exinf != src)
		end

		case (src.getbyte(0))
		# 異常発生あり/なしの場合
		when 0x41,0x42 then
			prop.set_exinf(src)
		# 0x41か0x42以外は受け付けない
		else
			return 0
		end

		return 1
	end

	# 現在年月日取得関数
	def date_prop_get(prop, size)
		# サイズが４以外は受け付けない
		if (size != 4) then
			return 0
		end

		time = Rtc.getTime

		return ((time[0] & 0xFF00) >> 8).chr + (time[0] & 0xFF).chr + time[1].chr + time[2].chr
	end

	# 現在年月日設定関数
	def date_prop_set(prop, src)
		# サイズが４以外は受け付けない
		if (src.bytesize != 4) then
			return 0
		end

		time = Rtc.getTime
		time[0] = (src.getbyte(0) << 8) | src.getbyte(1)
		time[1] = src.getbyte(2)
		time[2] = src.getbyte(3)
		Rtc.setTime time[0], time[1], time[2], time[3], time[4], time[5]

		return 4
	end

	# 現在時刻取得関数
	def time_prop_get(prop, size)
		# サイズが２以外は受け付けない
		if (size != 2) then
			return 0
		end

		time = Rtc.getTime

		return time[3].chr + time[4].chr
	end

	# 現在時刻設定関数
	def time_prop_set(prop, src)
		# サイズが２以外は受け付けない
		if (src.bytesize != 2) then
			return 0
		end

		time = Rtc.getTime
		time[3] = src.getbyte(0)
		time[4] = src.getbyte(1)
		Rtc.setTime time[0], time[1], time[2], time[3], time[4], time[5]

		return 2
	end
end

class Controller < ECNL::SvcTask
	def initialize()
		@profile = LocalNode.new(0x01)
		@devices = [ ControllerObj.new(0x01, @profile) ]
		@enodadrb_table = []

		super()

		# LEDをOFF
		pinMode(TargetBoard::LED_USER, 1)
		digitalWrite(TargetBoard::LED_USER, 0)
		digitalWrite(TargetBoard::LED_BLUE, 0)

		set_timer(1000)
	end

	def recv_esv(esv)
		if (esv.esv != ECNL::ESV_GET_RES) && (esv.esv != ECNL::ESV_GET_SNA) then
			return
		end

		itr = esv.itr_ini()
		itr.itr_nxt()
		until itr.is_eof do
			if itr.epc == 0xD6 then
				# LEDをON
				digitalWrite(TargetBoard::LED_USER, 1)
			end
			itr.itr_nxt()
		end
	end

	def break_wait(data)
	end

	def timeout()
		esv = esv_get(nil, 0xD6)
		snd_esv(esv)

		set_timer(10000)
	end

	def snd_msg(ep, data)
		# 通信レイヤーへ送信
		TargetBoard::snd_msg(ep, data)
	end

	def is_local_addr(ep)
		TargetBoard::is_local_addr(ep)
	end

	def is_multicast_addr(ep)
		TargetBoard::is_multicast_addr(ep)
	end

	def is_valid_addrid(id)
		(id >= 0) && ((id - ECNL::ENOD_REMOTE_ID) < @enodadrb_table.length)
	end

	def get_local_addr()
		TargetBoard::get_local_addr()
	end

	def get_multicast_addr()
		TargetBoard::get_multicast_addr()
	end

	def get_remote_addr(id)
		index = id - ECNL::ENOD_REMOTE_ID
		if (index < 0) || (index >= @enodadrb_table.length)
			nil
		else
			@enodadrb_table[index]
		end
	end

	# 通信レイヤーアドレスの同じものを検索
	def get_remote_id(ep)
		id = ECNL::ENOD_REMOTE_ID - 1
		for ea in @enodadrb_table do
			id += 1
			if !ea then
				next
			end
			if !TargetBoard::equals_addr(ea, ep) then
				next
			end

			return id
		end

		return ECNL::ENOD_NOT_MATCH_ID
	end

	# 対応するリモートノードを検索
	def set_remote_addr(edata, ep)
		id = ECNL::ENOD_REMOTE_ID - 1
		for ea in @enodadrb_table do
			id += 1
			if !ea then
				next
			end
			if !is_match(svc, edata, ep) then
				next
			end

			# 対応するリモートノードがあれば通信レイヤーアドレスを設定
			@enodadrb_table[id - ECNL::ENOD_REMOTE_ID] = ep

			return id
		end

		return ECNL::ENOD_NOT_MATCH_ID
	end

	# 空き領域を探して自動登録
	def add_remote_addr(edata, ep)
		id = ECNL::ENOD_REMOTE_ID - 1
		for ea in @enodadrb_table do
			id += 1
			if ea then
				next
			end

			@enodadrb_table[id - ECNL::ENOD_REMOTE_ID] = ep

			return id
		end

		if @enodadrb_table.length >= 100 then
			return ECNL::ENOD_NOT_MATCH_ID
		end

		id = @enodadrb_table.length;
		@enodadrb_table[id] = ep

		return id + ECNL::ENOD_REMOTE_ID
	end
end

ctrl = Controller.new()

# メインループ
while (true) do
	# メッセージ待ち
	ret = TargetBoard::wait_msg(ctrl.timer)
	if !ret then
		break;
	end

	# 経過時間の計算
	ctrl.progress ret[0]

	# 戻り値が２つなら
	if ret.length == 2 then
		# 内部イベント
		case (ret[1])
		when 1 then
			TargetBoard::restart
		when 2 then
			ctrl.ntf_inl
		end
	# 戻り値が３つなら
	elsif ret.length == 3 then
		# 通信レイヤーからのメッセージ（通信端点と電文）
		ctrl.recv_msg(ret[1], ret[2])
	end

	# タイムアウトの処理があれば行う
	ctrl.call_timeout
end
