import React, {useEffect, useRef, useState} from 'react'
import {
  SafeAreaView,
  StatusBar,
  StyleSheet,
  Text,
  useColorScheme,
  View,
  Pressable,
  ActivityIndicator,
  Platform,
  Alert,
  BackHandler,
  ListRenderItemInfo
} from 'react-native'

import RNBluetoothClassic, {BluetoothDevice} from 'react-native-bluetooth-classic'
import {FlatList} from 'react-native-gesture-handler'
import colors from '../../colors'
import {ScanProps} from '../../types'

function Scan({navigation}: ScanProps) {
  const [connecting, setConnecting] = useState(false)

  const isDarkMode = useColorScheme() === 'dark'

  const [deviceList, setDeviceList] = useState(Array<BluetoothDevice>)

  const canavar = useRef({} as BluetoothDevice)

  useEffect(() => {
    RNBluetoothClassic.onBluetoothEnabled(scan)
    ;(async function checkIsBluetoothAvailable() {
      let available: boolean = false
      try {
        available = await RNBluetoothClassic.isBluetoothAvailable()
      } catch (err) {
        console.log(err)
      }
      if (!available) {
        Alert.alert('블루투스가 필요합니다', '해당 기기는 블루투스를 지원하지 않습니다!', [
          {
            text: 'Exit',
            onPress: () => {
              if (Platform.OS === 'android') BackHandler.exitApp()
              else throw new Error('For exit')
            }
          }
        ])
      }
    })()
    ;(async function checkIsBluetoothEnabled() {
      let enabled: boolean = false
      try {
        enabled = await RNBluetoothClassic.isBluetoothEnabled()
      } catch (err) {
        console.log(err)
      }
      if (!enabled) {
        Platform.OS === 'android'
          ? Alert.alert('블루투스가 꺼져있습니다', '블루투스는 반드시 켜져있어야 합니다!', [
              {
                text: 'Turn On',
                onPress: () => {
                  try {
                    RNBluetoothClassic.requestBluetoothEnabled()
                  } catch (err) {
                    console.log(err)
                  }
                }
              }
            ])
          : Alert.alert('블루투스가 꺼져있습니다', '블루투스는 반드시 켜져있어야 합니다!', [
              {
                text: 'Exit',
                onPress: () => {
                  throw new Error('For exit')
                }
              }
            ])
      }
    })()

    scan()
  }, [])

  async function scan() {
    let paired
    try {
      paired = await RNBluetoothClassic.getBondedDevices()
    } catch (err) {
      console.log(err)
    }

    if (paired) {
      setDeviceList(paired)
    }
  }

  async function connect(selectedName: string) {
    let canavar1 = deviceList.find(device => device.name === selectedName)

    if (!canavar1) {
      return
    }

    if (connecting) {
      return
    }

    setConnecting(true)
    canavar.current = canavar1

    let isConnected: boolean = false

    try {
      isConnected = await canavar1.connect()
      RNBluetoothClassic.onDeviceDisconnected(onDisconnect)
    } catch (err) {
      console.log(err)
    }

    setConnecting(false)

    if (!isConnected) {
      Alert.alert('연결 실패')
      return
    }
    navigation.navigate('Controller', {deviceName: canavar1.name})
  }

  function onDisconnect() {
    Alert.alert('연결 실패', '다시 스캐닝 화면으로 돌아갑니다!', [
      {
        text: 'Ok',
        onPress: () => {
          navigation.navigate('Scan')
        }
      }
    ])
  }

  function renderItem({item}: ListRenderItemInfo<BluetoothDevice>) {
    return (
      <Pressable onPress={() => connect(item.name)} style={styles.deviceCard}>
        <Text style={styles.deviceName}>{item.name}</Text>
        <Text style={styles.deviceAddress}>{item.address}</Text>
      </Pressable>
    )
  }

  function seperator() {
    return <View style={styles.seperator} />
  }

  return (
    <SafeAreaView style={styles.mainPage}>
      <StatusBar barStyle={isDarkMode ? 'light-content' : 'dark-content'} />

      <View>
        <FlatList
          data={deviceList}
          renderItem={renderItem}
          keyExtractor={(_, index) => index.toString()}
          ItemSeparatorComponent={seperator}
          ListHeaderComponent={<Text style={styles.headerText}>페어링 된 디바이스</Text>}
          ListEmptyComponent={
            <Text style={styles.noDeviceText}>페어링된 디바이스를 찾지 못했습니다</Text>
          }
        />
      </View>
      <View style={styles.indicatorContainer}>
        {connecting && <ActivityIndicator size={Platform.OS === 'android' ? 70 : 'large'} />}
      </View>
    </SafeAreaView>
  )
}

const styles = StyleSheet.create({
  mainPage: {
    flex: 1,
    padding: 16,
    backgroundColor: colors.white
  },
  headerText: {
    fontSize: 26,
    fontWeight: '800',
    color: colors.water
  },
  seperator: {
    width: '100%',
    height: 2,
    backgroundColor: colors.dark
  },
  deviceCard: {
    paddingVertical: 8
  },
  deviceName: {
    fontSize: 24,
    fontWeight: '600',
    color: colors.dark
  },
  deviceAddress: {
    marginTop: 8,
    fontSize: 18,
    fontWeight: '400',
    color: colors.water
  },
  noDeviceText: {
    fontSize: 26,
    fontWeight: '800',
    color: colors.dark
  },
  indicatorContainer: {
    position: 'absolute',
    top: 0,
    left: 0,
    right: 0,
    bottom: 0,
    justifyContent: 'center',
    alignItems: 'center'
  }
})

export default Scan
