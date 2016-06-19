# TOPPERSプロジェクト
$MAKER_CODE = 0x00.chr + 0x00.chr + 0xB3.chr

# ノードプロファイルオブジェクト
class LocalNode < ECNL::ENode
	def initialize(eojx3)
		# 動作状態
		@operation_status = 0x30.chr
		# Ｖｅｒｓｉｏｎ情報
		@version_information = 0x01.chr + 0x0A.chr + 0x01.chr + 0x00.chr
		# 識別番号
		@identification_number = 
			# 下位通信層IDフィールド
			0xFE.chr +
			# メーカーコード
			$MAKER_CODE +
			# ユニークID部(メーカー独自)
			0x00.chr + 0x00.chr + 0x00.chr + 0x00.chr + 0x00.chr + 0x00.chr + 0x00.chr + 0x00.chr + 0x00.chr + 0x00.chr + 0x00.chr + 0x00.chr + 0x00.chr
		# 異常内容
		@fault_content = 0x00.chr + 0x00.chr
		# メーカーコード
		@manufacturer_code = $MAKER_CODE

		# インスタンス数
		@inst_count = 0x0.chr + 0x0.chr + 0x1.chr
		# クラス数
		@class_count = 0x0.chr + 0x2.chr
		# インスタンスリスト
		@inst_list = 0x1.chr + 0x5.chr + 0xff.chr + 0x1.chr
		# クラスリスト
		@class_list = 0x1.chr + 0x5.chr + 0xff.chr
		# アナウンスプロパティマップ
		@anno_prpmap = 0x1.chr + 0xd5.chr
		# SETプロパティマップ
		@set_prpmap = 0x1.chr + 0x80.chr
		# GETプロパティマップ
		@get_prpmap = 0xc.chr + 0x80.chr + 0x82.chr + 0x83.chr + 0x89.chr + 0x8a.chr + 0x9d.chr + 0x9e.chr + 0x9f.chr + 0xd3.chr + 0xd4.chr + 0xd6.chr + 0xd7.chr
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

	def node_profile_object_fault_content_set(prop, dst)
	end

	def onoff_prop_set(prop, dst)
	end
end

# コントローラークラス
class ControllerObj < ECNL::EObject
	def initialize(eojx3, enod)
		# 動作状態
		@operation_status = 0x30.chr
		# 設置場所
		@installation_location = 0x01.chr + 0x0A.chr + 0x01.chr + 0x00.chr
		# 規格Ｖｅｒｓｉｏｎ情報
		@standard_version_information = 0x00.chr + 0x00.chr + 'C'.chr + 0x00.chr
		# 異常発生状態
		@fault_status = 0x41.chr
		# メーカーコード
		@manufacturer_code = $MAKER_CODE

		# アナウンスプロパティマップ
		@anno_prpmap = 0x3.chr + 0x80.chr + 0x81.chr + 0x88.chr
		# SETプロパティマップ
		@set_prpmap = 0x4.chr + 0x80.chr + 0x81.chr + 0x97.chr + 0x98.chr
		# GETプロパティマップ
		@get_prpmap = 0xa.chr + 0x80.chr + 0x81.chr + 0x82.chr + 0x88.chr + 0x8a.chr + 0x97.chr + 0x98.chr + 0x9d.chr + 0x9e.chr + 0x9f.chr
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

	def onoff_prop_set(prop, dst)
	end

	def alarm_prop_set(prop, dst)
	end

	def date_prop_get(prop, size)
	end

	def date_prop_set(prop, dst)
	end

	def time_prop_get(prop, size)
	end

	def time_prop_set(prop, dst)
	end
end

class Controller < ECNL::SvcTask
	def initialize()
		@profile = LocalNode.new(0x01)
		@devices = [ ControllerObj.new(0x01, @profile) ]

		super()

		# 7segを"0"と表示
		TargetBoard::set_led(0xC0)

		@timer = 1000
	end

	def get_timer()
		timer = super()

		if timer < 0 || @timer < timer then
			timer = @timer
		end

		timer
	end

	def progress(interval)
		super(interval)

		@timer -= interval
		if @timer < 0 then
			@timer = 0
		end
	end

	def recv_esv(esv)
		if (esv.get_esv() != ECNL::ESV_GET_RES) && (esv.esv != ECNL::ESV_GET_SNA) then
			return
		end

		itr = esv.itr_ini()
		itr.itr_nxt()
		until itr.is_eof do
			if itr.get_epc() == 0xD6 then
				TargetBoard::set_led(0xF9)
			end
			itr.itr_nxt()
		end
	end

	def break_wait(data)
	end

	def timeout()
		super()

		if @timer == 0 then
			esv = esv_get(nil, 0xD6)
			snd_esv(esv)
			@timer = 10000
		end
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

	def equals_addr(ep1, ep2)
		TargetBoard::equals_addr(ep1, ep2)
	end

	def get_local_addr()
		TargetBoard::get_local_addr()
	end

	def get_multicast_addr()
		TargetBoard::get_multicast_addr()
	end
end

ctrl = Controller.new()
TargetBoard::set(ctrl)
